import { useState } from 'react';
import { useLanguage } from '../i18n/LanguageContext';

type ManualClientId = 'claude-desktop' | 'cursor';

const manualClients: { id: ManualClientId; tKey: string }[] = [
    { id: 'claude-desktop', tKey: 'mcp.client.claudeDesktop' },
    { id: 'cursor', tKey: 'mcp.client.cursor' },
];

const manualSnippets: Record<ManualClientId, string> = {
    'claude-desktop': `// ~/Library/Application Support/Claude/claude_desktop_config.json   (macOS)
// %APPDATA%/Claude/claude_desktop_config.json                       (Windows)
{
  "mcpServers": {
    "ymind": {
      "command": "ymind-cli",
      "args": ["mcp"]
    }
  }
}`,
    cursor: `// .cursor/mcp.json (project) or via Cursor → Settings → MCP
{
  "mcpServers": {
    "ymind": {
      "command": "ymind-cli",
      "args": ["mcp"]
    }
  }
}`,
};

const pluginCommands = `/plugin marketplace add broccoli-97/xmind
/plugin install ymind`;

const tools = [
    { name: 'render_mindmap', tKey: 'mcp.tool.render' },
    { name: 'list_layouts', tKey: 'mcp.tool.layouts' },
    { name: 'list_themes', tKey: 'mcp.tool.themes' },
];

function CopyButton({ text, theme = 'dark' }: { text: string; theme?: 'dark' | 'light' }) {
    const [copied, setCopied] = useState(false);
    const cls =
        theme === 'dark'
            ? 'border-white/15 bg-white/5 text-slate-300 hover:border-white/30 hover:text-white'
            : 'border-slate-300 bg-white text-slate-600 hover:border-slate-400 hover:text-slate-900';
    return (
        <button
            type="button"
            onClick={() => {
                navigator.clipboard.writeText(text).then(() => {
                    setCopied(true);
                    window.setTimeout(() => setCopied(false), 1500);
                });
            }}
            className={`rounded-md border px-2.5 py-1 text-xs font-medium transition-colors ${cls}`}
        >
            {copied ? '✓' : 'Copy'}
        </button>
    );
}

export function MCPGuide() {
    const { t } = useLanguage();
    const [activeManual, setActiveManual] = useState<ManualClientId>('claude-desktop');

    return (
        <section id="mcp" className="bg-white py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                {/* Header */}
                <div className="mb-16 text-center">
                    <div className="mb-4 inline-flex items-center gap-2 rounded-full border border-indigo-200 bg-indigo-50 px-3 py-1 text-xs font-medium text-indigo-700">
                        <span className="h-1.5 w-1.5 rounded-full bg-indigo-500" />
                        {t('mcp.badge')}
                    </div>
                    <h2 className="mb-4 text-4xl font-bold text-slate-900">{t('mcp.title')}</h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-600">
                        {t('mcp.subtitle')}
                    </p>
                </div>

                {/* Tools strip */}
                <div className="mb-16 grid gap-4 sm:grid-cols-3">
                    {tools.map((tool) => (
                        <div
                            key={tool.name}
                            className="rounded-xl border border-slate-200 bg-slate-50 p-5"
                        >
                            <div className="mb-2 font-mono text-sm font-semibold text-indigo-600">
                                {tool.name}
                            </div>
                            <p className="text-sm leading-relaxed text-slate-600">
                                {t(`${tool.tKey}.desc`)}
                            </p>
                        </div>
                    ))}
                </div>

                {/* Featured: Claude Code plugin install */}
                <div className="mb-12">
                    <div className="mb-6 flex items-center gap-3">
                        <span className="rounded-full bg-emerald-100 px-2.5 py-0.5 text-xs font-semibold text-emerald-700">
                            {t('mcp.install.recommendedTag')}
                        </span>
                        <h3 className="text-2xl font-semibold text-slate-900">
                            {t('mcp.install.pluginTitle')}
                        </h3>
                    </div>
                    <p className="mb-6 text-slate-600">{t('mcp.install.pluginBody')}</p>

                    <div className="overflow-hidden rounded-xl border-2 border-indigo-200 bg-gradient-to-br from-indigo-50 to-white">
                        <div className="flex items-center justify-between border-b border-indigo-100 bg-white/60 px-4 py-2">
                            <span className="font-mono text-xs font-medium text-indigo-700">
                                Claude Code
                            </span>
                            <CopyButton text={pluginCommands} theme="light" />
                        </div>
                        <pre className="overflow-x-auto p-6 font-mono text-sm leading-relaxed text-slate-900">
                            <code>{pluginCommands}</code>
                        </pre>
                    </div>

                    <p className="mt-4 text-sm text-slate-500">{t('mcp.install.pluginNote')}</p>
                </div>

                {/* One-time prerequisite: ymind-cli on PATH */}
                <div className="mb-12 rounded-xl border border-amber-200 bg-amber-50 p-6">
                    <h4 className="mb-2 flex items-center gap-2 font-semibold text-amber-900">
                        <svg className="h-5 w-5" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={2}>
                            <path strokeLinecap="round" strokeLinejoin="round" d="M12 9v3.75m-9.303 3.376c-.866 1.5.217 3.374 1.948 3.374h14.71c1.73 0 2.813-1.874 1.948-3.374L13.949 3.378c-.866-1.5-3.032-1.5-3.898 0L2.697 16.126zM12 15.75h.007v.008H12v-.008z" />
                        </svg>
                        {t('mcp.path.title')}
                    </h4>
                    <p className="mb-4 text-sm leading-relaxed text-amber-900">
                        {t('mcp.path.body')}
                    </p>
                    <div className="space-y-2 text-sm text-amber-900">
                        <div>
                            <span className="font-semibold">{t('mcp.path.linuxMac')}: </span>
                            <code className="ml-1 rounded bg-amber-100 px-1.5 py-0.5 font-mono text-xs">
                                sudo ln -s /path/to/ymind-cli /usr/local/bin/ymind-cli
                            </code>
                        </div>
                        <div>
                            <span className="font-semibold">{t('mcp.path.fromSource')}: </span>
                            <code className="ml-1 rounded bg-amber-100 px-1.5 py-0.5 font-mono text-xs">
                                export PATH="$PWD/build:$PATH"
                            </code>
                        </div>
                    </div>
                </div>

                {/* Manual JSON config for other clients */}
                <h3 className="mb-2 text-2xl font-semibold text-slate-900">
                    {t('mcp.manual.title')}
                </h3>
                <p className="mb-6 text-slate-600">{t('mcp.manual.body')}</p>

                <div className="overflow-hidden rounded-xl border border-slate-800 bg-slate-900">
                    <div className="flex items-center justify-between border-b border-slate-800 bg-slate-950/40 px-3 py-2">
                        <div className="flex gap-1">
                            {manualClients.map((c) => (
                                <button
                                    key={c.id}
                                    onClick={() => setActiveManual(c.id)}
                                    className={`rounded-md px-3 py-1.5 text-xs font-medium transition-colors ${
                                        activeManual === c.id
                                            ? 'bg-indigo-500/20 text-indigo-300'
                                            : 'text-slate-400 hover:text-white'
                                    }`}
                                >
                                    {t(c.tKey)}
                                </button>
                            ))}
                        </div>
                        <CopyButton text={manualSnippets[activeManual]} />
                    </div>
                    <pre className="overflow-x-auto p-6 text-sm leading-relaxed text-slate-100">
                        <code>{manualSnippets[activeManual]}</code>
                    </pre>
                </div>

                {/* Try it */}
                <h3 className="mb-2 mt-16 text-2xl font-semibold text-slate-900">
                    {t('mcp.example.title')}
                </h3>
                <p className="mb-6 text-slate-600">{t('mcp.example.body')}</p>

                <div className="overflow-hidden rounded-xl border border-slate-200 bg-slate-50">
                    <div className="border-b border-slate-200 bg-white px-4 py-2 text-xs font-medium text-slate-500">
                        {t('mcp.example.youSay')}
                    </div>
                    <div className="px-6 py-5 text-slate-800">
                        “{t('mcp.example.prompt')}”
                    </div>
                </div>

                {/* Build hint */}
                <div className="mt-10 rounded-xl border border-slate-200 bg-slate-50 px-5 py-4 text-sm text-slate-700">
                    <strong className="font-semibold">{t('mcp.build.label')} </strong>
                    {t('mcp.build.body')}
                    <code className="ml-2 rounded bg-slate-200 px-1.5 py-0.5 font-mono text-xs">
                        cmake -B build && cmake --build build
                    </code>
                </div>
            </div>
        </section>
    );
}
