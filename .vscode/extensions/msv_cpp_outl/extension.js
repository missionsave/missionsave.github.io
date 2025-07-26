const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

class EnhancedOutlineProvider {
    constructor(context) {
        this._onDidChangeTreeData = new vscode.EventEmitter();
        this.onDidChangeTreeData = this._onDidChangeTreeData.event;
        this._highlightedItem = null;
        this._currentItems = [];
        this._currentSymbols = [];
        this._context = context;
        this._outputChannel = vscode.window.createOutputChannel('Enhanced Outline Debug');
    }

    // ==================== NOVOS MÉTODOS ADICIONADOS ====================
    showExtensionInfo() {
        try {
            const extensionPath = path.join(this._context.extensionPath, 'extension.js');
            const stats = fs.statSync(extensionPath);
            const modifiedTime = stats.mtime;
            const now = new Date();
            const diffSeconds = Math.round((now - modifiedTime) / 1000);

            this._outputChannel.clear();
            this._outputChannel.appendLine('=== INFORMAÇÕES DA EXTENSÃO ===');
            this._outputChannel.appendLine(`📂 Arquivo: ${path.basename(extensionPath)}`);
            this._outputChannel.appendLine(`📅 Modificado em: ${modifiedTime.toLocaleString()}`);
            this._outputChannel.appendLine(`🕒 Carregado em: ${now.toLocaleString()}`);
            this._outputChannel.appendLine(`⏳ Diferença: ${diffSeconds} segundos`);
            this._outputChannel.appendLine(diffSeconds > 30 ? '⚠️ ATENÇÃO: Possível versão em cache!' : '✅ Extensão atualizada');
            this._outputChannel.appendLine('==============================');
            this._outputChannel.show(true);
        } catch (error) {
            this._outputChannel.appendLine(`❌ Erro: ${error.message}`);
        }
    }

    forceReload() {
        this.showExtensionInfo();
        setTimeout(() => vscode.commands.executeCommand('workbench.action.reloadWindow'), 300);
    }
    // ====================================================================

    // ... (MÉTODOS EXISTENTES DA SUA EXTENSÃO) ...
    refresh() {
        this._onDidChangeTreeData.fire();
    }

    getTreeItem(element) {
        if (!element) return new vscode.TreeItem('No elements found');
        element.description = this._highlightedItem === element ? '←' : '';
        return element;
    }

    async getChildren(element) {
        const editor = vscode.window.activeTextEditor;
        if (!editor) return [];

        const document = editor.document;
        const symbols = await this.getDocumentSymbols(document);
        const regions = this.parseRegions(document);
        this._currentSymbols = symbols;

        if (!element) {
            const items = [];
            const symbolInsideAnyRegionDeep = (symbol, regions) => {
                for (const region of regions) {
                    if (this.symbolInsideRegion(symbol, region.range)) return true;
                    if (region.children && symbolInsideAnyRegionDeep(symbol, region.children)) return true;
                }
                return false;
            };

            for (const symbol of symbols) {
                if (!symbolInsideAnyRegionDeep(symbol, regions)) {
                    items.push(this.createSymbolItem(symbol));
                }
            }

            for (const region of regions) {
                if (!this.regionInsideAnySymbol(region.range, symbols)) {
                    items.push(this.createRegionItem(region));
                }
            }

            this._currentItems = items.sort((a, b) => a.range.start.line - b.range.start.line);
            return this._currentItems;
        }

        if (element.contextValue === 'region') {
            const children = [];
            if (element.region?.children) {
                for (const region of element.region.children) {
                    children.push(this.createRegionItem(region));
                }
            }

            const collectSymbolsInRegion = (symbols, range, out = []) => {
                for (const s of symbols) {
                    if (this.symbolInsideRegion(s, range)) {
                        out.push(this.createSymbolItem(s));
                    }
                    if (s.children) {
                        collectSymbolsInRegion(s.children, range, out);
                    }
                }
                return out;
            };

            collectSymbolsInRegion(this._currentSymbols || [], element.range, children);
            return children.sort((a, b) => a.range.start.line - b.range.start.line);
        }

        if (element.contextValue === 'symbol') {
            const children = [];
            const regionList = this.parseRegions(document);
            const childSymbols = element.symbol?.children || [];
            const filteredChildren = childSymbols.filter(child => !this.symbolInsideAnyRegion(child, regionList));

            for (const region of regionList) {
                if (this.symbolInsideRegion({ range: region.range }, element.range)) {
                    children.push(this.createRegionItem(region));
                }
            }

            for (const child of filteredChildren) {
                children.push(this.createSymbolItem(child));
            }

            return children.sort((a, b) => a.range.start.line - b.range.start.line);
        }

        return [];
    }

    async getDocumentSymbols(document) {
        try {
            const symbols = await vscode.commands.executeCommand(
                'vscode.executeDocumentSymbolProvider', 
                document.uri
            );
            return Array.isArray(symbols) ? symbols : [];
        } catch (e) {
            console.error('Error getting symbols:', e);
            return [];
        }
    }

    parseRegions(document) {
        const REGION_REGEX = /^\s*#pragma\s+region\s+(.*)$/;
        const ENDREGION_REGEX = /^\s*#pragma\s+endregion\b/;
        const regions = [];
        const stack = [];
        let count = 0;

        for (let line = 0; line < document.lineCount; line++) {
            const text = document.lineAt(line).text;
            const startMatch = text.match(REGION_REGEX);
            if (startMatch) {
                const name = startMatch[1]?.trim() || `Region ${++count}`;
                const region = { 
                    name, 
                    rangeStart: line, 
                    rangeEnd: line, 
                    children: [] 
                };
                if (stack.length > 0) stack[stack.length - 1].children.push(region);
                else regions.push(region);
                stack.push(region);
                continue;
            }
            if (ENDREGION_REGEX.test(text)) {
                if (stack.length > 0) {
                    const region = stack.pop();
                    region.rangeEnd = line;
                }
            }
        }

        const wrap = r => ({
            name: r.name,
            range: new vscode.Range(
                new vscode.Position(r.rangeStart, 0), 
                new vscode.Position(r.rangeEnd, 1000)
            ),
            children: r.children.map(wrap),
            raw: r
        });

        return regions.map(wrap);
    }

    createRegionItem(region) {
        const item = new vscode.TreeItem(
            region.name, 
            vscode.TreeItemCollapsibleState.Collapsed
        );
        item.iconPath = new vscode.ThemeIcon('symbol-namespace');
        item.contextValue = 'region';
        item.range = region.range;
        item.region = region;
        item.command = {
            command: 'enhancedOutline.revealRange',
            title: 'Go to Region',
            arguments: [region.range]
        };
        return item;
    }

    createSymbolItem(symbol) {
        const item = new vscode.TreeItem(
            symbol.name,
            symbol.children?.length > 0 
                ? vscode.TreeItemCollapsibleState.Collapsed 
                : vscode.TreeItemCollapsibleState.None
        );
        item.iconPath = this.getSymbolIcon(symbol.kind);
        item.contextValue = 'symbol';
        item.range = symbol.range;
        item.symbol = symbol;
        item.command = {
            command: 'enhancedOutline.revealRange',
            title: 'Go to Symbol',
            arguments: [symbol.range]
        };
        return item;
    }

    getSymbolIcon(kind) {
        const iconMap = {
            [vscode.SymbolKind.File]: 'file',
            [vscode.SymbolKind.Module]: 'file-submodule',
            [vscode.SymbolKind.Namespace]: 'symbol-namespace',
            [vscode.SymbolKind.Package]: 'package',
            [vscode.SymbolKind.Class]: 'symbol-class',
            [vscode.SymbolKind.Method]: 'symbol-method',
            [vscode.SymbolKind.Property]: 'symbol-property',
            [vscode.SymbolKind.Field]: 'symbol-field',
            [vscode.SymbolKind.Constructor]: 'symbol-constructor',
            [vscode.SymbolKind.Enum]: 'symbol-enum',
            [vscode.SymbolKind.Interface]: 'symbol-interface',
            [vscode.SymbolKind.Function]: 'symbol-function',
            [vscode.SymbolKind.Variable]: 'symbol-variable',
            [vscode.SymbolKind.Constant]: 'symbol-constant',
            [vscode.SymbolKind.String]: 'symbol-string',
            [vscode.SymbolKind.Number]: 'symbol-number',
            [vscode.SymbolKind.Boolean]: 'symbol-boolean',
            [vscode.SymbolKind.Array]: 'symbol-array',
            [vscode.SymbolKind.Object]: 'symbol-object',
            [vscode.SymbolKind.Key]: 'symbol-key',
            [vscode.SymbolKind.Null]: 'symbol-null',
            [vscode.SymbolKind.EnumMember]: 'symbol-enum-member',
            [vscode.SymbolKind.Struct]: 'symbol-structure',
            [vscode.SymbolKind.Event]: 'symbol-event',
            [vscode.SymbolKind.Operator]: 'symbol-operator',
            [vscode.SymbolKind.TypeParameter]: 'symbol-type-parameter'
        };
        return new vscode.ThemeIcon(iconMap[kind] || 'symbol-misc');
    }

    symbolInsideRegion(symbol, range) {
        return symbol.range.start.line > range.start.line && 
               symbol.range.end.line < range.end.line;
    }

    symbolInsideAnyRegion(symbol, regions) {
        for (const region of regions) {
            if (this.symbolInsideRegion(symbol, region.range)) return true;
            if (region.children && this.symbolInsideAnyRegion(symbol, region.children)) return true;
        }
        return false;
    }

    regionInsideAnySymbol(regionRange, symbols) {
        for (const symbol of symbols) {
            if (regionRange.start.line > symbol.range.start.line && 
                regionRange.end.line < symbol.range.end.line) {
                return true;
            }
            if (symbol.children && this.regionInsideAnySymbol(regionRange, symbol.children)) {
                return true;
            }
        }
        return false;
    }

    updateHighlightingForPosition(pos) {
        const previous = this._highlightedItem;
        this._highlightedItem = null;

        for (const item of this._currentItems) {
            if (item.range?.contains(pos)) {
                this._highlightedItem = item;
                break;
            }
        }

        if (this._highlightedItem !== previous) {
            this.refresh();
        }
    }
}

function activate(context) {
    const provider = new EnhancedOutlineProvider(context);
    
    // Mostra informações ao carregar
    provider.showExtensionInfo();

    // Cria botão na barra de status
    const reloadButton = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Right, 100);
    reloadButton.text = '$(debug-restart) Recarregar';
    reloadButton.tooltip = 'Forçar recarregamento da extensão';
    reloadButton.command = 'enhancedOutline.forceReload';
    reloadButton.show();

    const treeView = vscode.window.createTreeView('enhancedOutline', {
        treeDataProvider: provider,
        showCollapseAll: true
    });

    context.subscriptions.push(
        reloadButton,
        treeView,
        vscode.commands.registerCommand('enhancedOutline.forceReload', () => {
            vscode.window.showInformationMessage('Recarregando extensão...');
            provider.forceReload();
        }),
        vscode.commands.registerCommand('enhancedOutline.showInfo', () => {
            provider.showExtensionInfo();
        }),
        vscode.commands.registerCommand('enhancedOutline.refresh', () => provider.refresh()),
        vscode.commands.registerCommand('enhancedOutline.revealRange', async (range) => {
            const editor = vscode.window.activeTextEditor;
            if (editor && range) {
                editor.selection = new vscode.Selection(range.start, range.start);
                editor.revealRange(range, vscode.TextEditorRevealType.InCenter);
            }
        }),
        vscode.window.onDidChangeTextEditorSelection(e => {
            if (e.textEditor === vscode.window.activeTextEditor && e.selections.length > 0) {
                provider.updateHighlightingForPosition(e.selections[0].active);
            }
        }),
        vscode.workspace.onDidChangeTextDocument(() => provider.refresh()),
        vscode.window.onDidChangeActiveTextEditor(() => provider.refresh())
    );

    // Força mostrar o output após 1 segundo
    setTimeout(() => {
        provider._outputChannel.show(true);
    }, 1000);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};