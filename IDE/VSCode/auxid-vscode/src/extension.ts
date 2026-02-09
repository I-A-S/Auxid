import * as vscode from 'vscode';
import * as cp from 'child_process';
import * as path from 'path';
import * as fs from 'fs';

import { AuxidSidebarProvider } from './sidebar_provider';

interface AuxidIssue {
    file: string;
    line: number;
    startCol: number;
    endCol: number;
    msg: string;
}

let diagnosticCollection: vscode.DiagnosticCollection;
let outputChannel: vscode.OutputChannel;
let sidebarProvider: AuxidSidebarProvider;
let isLinterEnabled = true;

export function activate(context: vscode.ExtensionContext) {
    outputChannel = vscode.window.createOutputChannel("Auxid");
    diagnosticCollection = vscode.languages.createDiagnosticCollection('auxid');
    context.subscriptions.push(diagnosticCollection);

    sidebarProvider = new AuxidSidebarProvider(context.extensionUri);
    context.subscriptions.push(
        vscode.window.registerWebviewViewProvider("auxid.sidebar", sidebarProvider)
    );

    context.subscriptions.push(vscode.commands.registerCommand('auxid.toggle', () => {
        isLinterEnabled = !isLinterEnabled;
        sidebarProvider.updateStatus(isLinterEnabled);

        if (!isLinterEnabled) {
            diagnosticCollection.clear();
            vscode.window.showInformationMessage('Auxid Validator Disabled (Window)');
        } else {
            vscode.window.showInformationMessage('Auxid Validator Enabled');
            if (vscode.window.activeTextEditor) {
                runLinter(vscode.window.activeTextEditor.document);
            }
        }
    }));

    context.subscriptions.push(vscode.commands.registerCommand('auxid.analyzeWorkspace', async () => {
        const excludePattern = '**/{out,build,_deps,deps,IACore,CMakeFiles,Vendor,External,node_modules,.git}/**';
        const files = await vscode.workspace.findFiles('**/*.{cpp,h,hpp,c}', excludePattern);

        let totalIssues = 0;
        let allIssues: AuxidIssue[] = [];

        await vscode.window.withProgress({
            location: vscode.ProgressLocation.Notification,
            title: "Auxid: Analyzing Workspace...",
            cancellable: true
        }, async (progress, token) => {
            const step = 100 / files.length;

            for (const file of files) {
                if (token.isCancellationRequested) {
                    break;
                }

                const doc = await vscode.workspace.openTextDocument(file);
                const result = await runValidatorCli(doc);

                if (result.issueCount > 0) {
                    totalIssues += result.issueCount;
                    allIssues.push(...result.issues);
                }
                progress.report({ increment: step, message: path.basename(file.fsPath) });
            }
        });

        sidebarProvider.reportResults({
            total: files.length,
            issueCount: totalIssues,
            issues: allIssues
        });
    }));

    context.subscriptions.push(
        vscode.workspace.onDidSaveTextDocument(document => {
            if (document.languageId === 'cpp' || document.languageId === 'c') {
                if (isLinterEnabled) { runLinter(document); }
            }
        })
    );
    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(document => {
            if (document.languageId === 'cpp' || document.languageId === 'c') {
                if (isLinterEnabled) { runLinter(document); }
            }
        })
    );
}

function getClangIncludePath(): string | null {
    try {
        const out = cp.execSync('clang -print-resource-dir', { encoding: 'utf8' }).trim();
        const includePath = path.join(out, 'include');
        if (fs.existsSync(includePath)) { return includePath; }
    } catch (e) {
        outputChannel.appendLine(`[Error] Could not find clang resource dir: ${e}`);
    }
    return null;
}

function runValidatorCli(document: vscode.TextDocument): Promise<{ issueCount: number, issues: AuxidIssue[] }> {
    return new Promise((resolve) => {
        const config = vscode.workspace.getConfiguration('auxid');
        const validatorPath = config.get<string>('validatorPath') || 'auxid-validator';
        const buildPathSetting = config.get<string>('buildPath');

        if (!validatorPath) {
            outputChannel.appendLine("[Error] Validator path not set.");
            resolve({ issueCount: 0, issues: [] });
            return;
        }

        let buildPath = buildPathSetting || '.';
        if (vscode.workspace.workspaceFolders) {
            const root = vscode.workspace.workspaceFolders[0].uri.fsPath;
            buildPath = buildPath.replace('${workspaceFolder}', root);
        }

        const existingDiagnostics = vscode.languages.getDiagnostics(document.uri);
        const hasCompilerErrors = existingDiagnostics.some(d =>
            (d.source === 'c++' || d.source === 'clang' || d.source === 'gcc') &&
            d.severity === vscode.DiagnosticSeverity.Error
        );

        if (hasCompilerErrors) {
            diagnosticCollection.delete(document.uri);
            resolve({ issueCount: 0, issues: [] });
            return;
        }

        const clangInclude = getClangIncludePath();
        const args = ['-p', buildPath, document.fileName];

        if (clangInclude) {
            args.push(`--extra-arg=-I${clangInclude}`);
        }

        cp.execFile(validatorPath, args, { cwd: path.dirname(document.fileName) }, (err, stdout, stderr) => {
            if (err && (err as any).code === 'ENOENT') {
                vscode.window.showErrorMessage(`Auxid Validator not found at: ${validatorPath}`);
                resolve({ issueCount: 0, issues: [] });
                return;
            }

            const lines = stdout.split('\n');
            const regex = /^.+:(\d+):(\d+):(\d+):\s+\[Auxid\]\s+Violation:\s+(.+)$/;

            const foundIssues: AuxidIssue[] = [];
            const diagnostics: vscode.Diagnostic[] = [];

            lines.forEach(line => {
                const match = line.match(regex);
                if (match) {
                    const lineNum = parseInt(match[1]) - 1;
                    const startColNum = parseInt(match[2]) - 1;
                    const endColNum = parseInt(match[3]) - 1;
                    const msg = match[4];

                    const endCol = (startColNum === endColNum) ? Number.MAX_SAFE_INTEGER : endColNum;

                    foundIssues.push({
                        file: document.fileName,
                        line: lineNum,
                        startCol: startColNum,
                        endCol: endCol,
                        msg: msg
                    });

                    if (isLinterEnabled) {
                        const range = new vscode.Range(lineNum, startColNum, lineNum, endCol);
                        const diagnostic = new vscode.Diagnostic(
                            range,
                            msg,
                            vscode.DiagnosticSeverity.Error
                        );
                        diagnostic.source = 'Auxid';
                        diagnostics.push(diagnostic);
                    }
                }
            });

            if (isLinterEnabled) {
                diagnosticCollection.set(document.uri, diagnostics);
            }

            resolve({ issueCount: foundIssues.length, issues: foundIssues });
        });
    });
}

async function runLinter(document: vscode.TextDocument) {
    if (!isLinterEnabled) { return; }
    runValidatorCli(document);
}

export function deactivate() { }