import * as vscode from "vscode";

export class OxideSidebarProvider implements vscode.WebviewViewProvider {
  _view?: vscode.WebviewView;

  constructor(private readonly _extensionUri: vscode.Uri) {}

  public resolveWebviewView(webviewView: vscode.WebviewView) {
    this._view = webviewView;

    webviewView.webview.options = {
      enableScripts: true,
      localResourceRoots: [this._extensionUri],
    };

    webviewView.webview.html = this._getHtmlForWebview(webviewView.webview);

    webviewView.webview.onDidReceiveMessage(async (data) => {
      switch (data.type) {
        case "onToggle": {
          vscode.commands.executeCommand("oxide.toggle");
          break;
        }
        case "onAnalyze": {
          vscode.commands.executeCommand("oxide.analyzeWorkspace");
          break;
        }
        case "onJump": {
            try {
                const doc = await vscode.workspace.openTextDocument(data.file);
                const editor = await vscode.window.showTextDocument(doc);
                const range = new vscode.Range(data.line, data.col, data.line, data.col);
                editor.selection = new vscode.Selection(data.line, data.col, data.line, data.col);
                editor.revealRange(range, vscode.TextEditorRevealType.InCenter);
            } catch (e) {
                vscode.window.showErrorMessage(`Could not open file: ${data.file}`);
            }
            break;
        }
      }
    });
  }

  public reportResults(stats: { total: number; issueCount: number; issues: any[] }) {
    this._view?.webview.postMessage({ type: "report", ...stats });
  }

  public updateStatus(isEnabled: boolean) {
    this._view?.webview.postMessage({ type: "status", value: isEnabled });
  }

  private _getHtmlForWebview(webview: vscode.Webview) {
    return `<!DOCTYPE html>
      <html lang="en">
      <head>
        <style>
          body { font-family: var(--vscode-font-family); padding: 10px; }
          button { 
            width: 100%; padding: 8px; margin-bottom: 10px; 
            background: var(--vscode-button-background); 
            color: var(--vscode-button-foreground); 
            border: none; cursor: pointer; 
          }
          button:hover { background: var(--vscode-button-hoverBackground); }
          .status { margin-bottom: 15px; font-weight: bold; }
          .green { color: #4EC9B0; }
          .red { color: #F14C4C; }
          
          #result-list { margin-top: 15px; }
          .issue-item {
            padding: 8px;
            margin-bottom: 5px;
            border-left: 3px solid #F14C4C;
            background-color: var(--vscode-editor-background);
            cursor: pointer;
            font-size: 0.9em;
          }
          .issue-item:hover {
            background-color: var(--vscode-list-hoverBackground);
          }
          .file-name { font-weight: bold; display: block; margin-bottom: 2px; }
          .issue-msg { opacity: 0.8; }
        </style>
      </head>
      <body>
        <div class="status">Validator Status: <span id="status-text">Enabled</span></div>
        <button onclick="post('onToggle')">Toggle Validator (Ctrl+Q)</button>
        <button onclick="post('onAnalyze')">Analyze Workspace</button>
        
        <div id="summary"></div>
        <div id="result-list"></div>

        <script>
          const vscode = acquireVsCodeApi();
          function post(type) { vscode.postMessage({ type: type }); }

          let currentIssues = [];

          function renderIssues(issues) {
            const container = document.getElementById('result-list');
            container.innerHTML = '';
            
            issues.forEach((issue, index) => {
                const div = document.createElement('div');
                div.className = 'issue-item';
                
                const fileName = issue.file.replace(/^.*[\\\\/]/, '');
                
                div.innerHTML = \`<span class="file-name">\${fileName}:\${issue.line + 1}</span><span class="issue-msg">\${issue.msg}</span>\`;
                
                div.onclick = () => {
                    vscode.postMessage({ 
                        type: 'onJump', 
                        file: issue.file, 
                        line: issue.line, 
                        col: issue.col 
                    });
                };
                container.appendChild(div);
            });
          }

          window.addEventListener('message', event => {
            const message = event.data;
            switch (message.type) {
              case 'status':
                const el = document.getElementById('status-text');
                el.innerText = message.value ? "Enabled" : "Disabled";
                el.style.color = message.value ? "inherit" : "gray";
                break;
                
              case 'report':
                const summary = document.getElementById('summary');
                if (message.issueCount === 0) {
                  summary.innerHTML = '<h3 class="green">All Good!</h3>Scanned ' + message.total + ' files.';
                  document.getElementById('result-list').innerHTML = '';
                } else {
                  summary.innerHTML = '<h3 class="red">' + message.issueCount + ' Warnings Found</h3>';
                  renderIssues(message.issues);
                }
                break;
            }
          });
        </script>
      </body>
      </html>`;
  }
}