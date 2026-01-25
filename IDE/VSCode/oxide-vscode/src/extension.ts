import * as vscode from 'vscode';
import * as cp from 'child_process';
import * as path from 'path';
import * as fs from 'fs';
 
let diagnosticCollection: vscode.DiagnosticCollection;
let outputChannel: vscode.OutputChannel;

export function activate(context: vscode.ExtensionContext) {
    outputChannel = vscode.window.createOutputChannel("Oxide");
    diagnosticCollection = vscode.languages.createDiagnosticCollection('oxide');
    context.subscriptions.push(diagnosticCollection);
     
    context.subscriptions.push(
        vscode.workspace.onDidSaveTextDocument(document => {
            if (document.languageId === 'cpp' || document.languageId === 'c') {
                runLinter(document);
            }
        })
    );
     
    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(document => {
            if (document.languageId === 'cpp' || document.languageId === 'c') {
                runLinter(document);
            }
        })
    );
}

function getClangIncludePath(): string | null {
    try {
        const out = cp.execSync('clang -print-resource-dir', { encoding: 'utf8' }).trim();
        const includePath = path.join(out, 'include');
        if (fs.existsSync(includePath)) {
            return includePath;
        }
    } catch (e) {
        outputChannel.appendLine(`[Error] Could not find clang resource dir: ${e}`);
    }
    return null;
}

 
async function runLinter(document: vscode.TextDocument) {
    const config = vscode.workspace.getConfiguration('oxide');
    const validatorPath = config.get<string>('validatorPath');
    const buildPathSetting = config.get<string>('buildPath');

    if (!validatorPath) {
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
        return; 
    }
     
    const clangInclude = getClangIncludePath();
    const args = ['-p', buildPath, document.fileName];
    
    if (clangInclude) {
        args.push(`--extra-arg=-I${clangInclude}`);
    }

    args.push('--raw');
     
    cp.execFile(validatorPath, args, { cwd: buildPath }, (err, stdout, stderr) => {
        if (err && (err as any).code === 'ENOENT') {
            vscode.window.showErrorMessage(`Oxide Validator not found at: ${validatorPath}`);
            return;
        }
         
        const diagnostics: vscode.Diagnostic[] = [];
        const lines = stdout.split('\n');

        const regex = /^(.+):(\d+):(\d+):\s+\[Oxide\]\s+Violation:\s+(.+)$/;

        for (const line of lines) {
            const match = line.match(regex);
            if (match) {
                 
                const lineNum = parseInt(match[2]) - 1;  
                const colNum = parseInt(match[3]) - 1;
                const msg = match[4];

                const range = new vscode.Range(lineNum, colNum, lineNum, 100);
                const diagnostic = new vscode.Diagnostic(
                    range, 
                    msg, 
                    vscode.DiagnosticSeverity.Warning  
                );
                diagnostic.source = 'Oxide';
                diagnostics.push(diagnostic);
            }
        }
         
        diagnosticCollection.set(document.uri, diagnostics);
    });
}

export function deactivate() {}