import { useEffect, useState } from 'react';
import { useLanguage } from '../i18n/LanguageContext';

type ThemePreview = {
    palette: string[];
    fillAlpha?: number;
    background?: string;
};

type ThemeEntry = {
    id: string;
    name: string;
    description: string;
    folder: string;
    preview: ThemePreview;
};

type Manifest = {
    themes: ThemeEntry[];
};

const THEMES_BASE = `${import.meta.env.BASE_URL}themes/`;

// A small mini mind-map preview that paints in the theme's actual palette so
// users can compare themes side by side instead of staring at name + text.
function MiniMap({ preview }: { preview: ThemePreview }) {
    const palette =
        preview.palette && preview.palette.length > 0
            ? preview.palette
            : ['#6366f1', '#8b5cf6', '#ec4899', '#f59e0b', '#10b981'];
    const fillAlpha = preview.fillAlpha ?? 0.6;
    const bg = preview.background ?? '#FFFFFF';

    // Four branches arrayed around a central "root" node.
    const branches = [
        { x: 34, y: 26 },
        { x: 34, y: 74 },
        { x: 166, y: 26 },
        { x: 166, y: 74 },
    ];

    return (
        <svg viewBox="0 0 200 100" className="h-32 w-full" style={{ background: bg }}>
            {branches.map((b, i) => {
                const c = palette[i % palette.length];
                const cx1 = i < 2 ? 80 : 120;
                const cx2 = i < 2 ? b.x + 18 : b.x - 18;
                return (
                    <path
                        key={`e${i}`}
                        d={`M 100 50 C ${cx1} 50, ${cx2} ${b.y}, ${b.x} ${b.y}`}
                        fill="none"
                        stroke={c}
                        strokeWidth="1.6"
                        strokeLinecap="round"
                    />
                );
            })}

            <rect
                x="78"
                y="40"
                width="44"
                height="20"
                rx="5"
                fill={palette[0]}
                fillOpacity={Math.min(1, fillAlpha + 0.18)}
                stroke={palette[0]}
                strokeWidth="1.4"
            />
            {branches.map((b, i) => {
                const c = palette[i % palette.length];
                return (
                    <rect
                        key={`n${i}`}
                        x={b.x - 18}
                        y={b.y - 8}
                        width="36"
                        height="16"
                        rx="4"
                        fill={c}
                        fillOpacity={fillAlpha}
                        stroke={c}
                        strokeWidth="1.2"
                    />
                );
            })}
        </svg>
    );
}

export function Themes() {
    const { t } = useLanguage();
    const [themes, setThemes] = useState<ThemeEntry[]>([]);
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        fetch(`${THEMES_BASE}manifest.json`)
            .then((r) => {
                if (!r.ok) throw new Error(`HTTP ${r.status}`);
                return r.json() as Promise<Manifest>;
            })
            .then((m) => setThemes(m.themes ?? []))
            .catch((e) => setError(String(e)));
    }, []);

    return (
        <section id="themes" className="bg-white py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="mb-16 text-center">
                    <h2 className="mb-4 text-4xl font-bold text-slate-900">
                        {t('themes.title')}
                    </h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-600">
                        {t('themes.subtitle')}
                    </p>
                </div>

                <div className="mb-10 rounded-xl border border-slate-200 bg-slate-50 p-6">
                    <h3 className="mb-2 text-base font-semibold text-slate-900">
                        {t('themes.howto.title')}
                    </h3>
                    <p className="text-sm leading-relaxed text-slate-600">
                        {t('themes.howto.body')}
                    </p>
                    <ol className="mt-3 list-inside list-decimal space-y-1 text-sm text-slate-600">
                        <li>{t('themes.howto.step1')}</li>
                        <li>{t('themes.howto.step2')}</li>
                        <li>{t('themes.howto.step3')}</li>
                    </ol>
                </div>

                {error && (
                    <p className="mb-6 rounded-md border border-amber-200 bg-amber-50 p-3 text-sm text-amber-800">
                        {t('themes.loaderror')}: {error}
                    </p>
                )}

                <div className="grid gap-6 sm:grid-cols-2 lg:grid-cols-3">
                    {themes.map((th) => (
                        <div
                            key={th.id}
                            className="overflow-hidden rounded-xl border border-slate-200 bg-white shadow-sm transition-shadow hover:shadow-md"
                        >
                            <div className="border-b border-slate-100">
                                <MiniMap preview={th.preview} />
                            </div>
                            <div className="p-5">
                                <h3 className="mb-1 text-lg font-semibold text-slate-900">
                                    {th.name}
                                </h3>
                                <p className="mb-4 text-sm leading-relaxed text-slate-600">
                                    {th.description}
                                </p>
                                <a
                                    href={`${THEMES_BASE}${th.folder}/theme.json`}
                                    download="theme.json"
                                    className="inline-flex items-center gap-1.5 rounded-md bg-indigo-600 px-3.5 py-2 text-sm font-medium text-white transition-colors hover:bg-indigo-500"
                                >
                                    <svg
                                        className="h-4 w-4"
                                        fill="none"
                                        viewBox="0 0 24 24"
                                        stroke="currentColor"
                                        strokeWidth={2}
                                    >
                                        <path
                                            strokeLinecap="round"
                                            strokeLinejoin="round"
                                            d="M3 16.5v2.25A2.25 2.25 0 005.25 21h13.5A2.25 2.25 0 0021 18.75V16.5M16.5 12L12 16.5m0 0L7.5 12m4.5 4.5V3"
                                        />
                                    </svg>
                                    {t('themes.download')}
                                </a>
                            </div>
                        </div>
                    ))}
                </div>
            </div>
        </section>
    );
}
