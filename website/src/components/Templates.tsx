import { useEffect, useState } from 'react';
import { useLanguage } from '../i18n/LanguageContext';

type TemplateEntry = {
    id: string;
    name: string;
    description: string;
    file: string;
    previewColors?: string[];
};

type Manifest = {
    templates: TemplateEntry[];
};

const TEMPLATES_BASE = `${import.meta.env.BASE_URL}templates/`;

function PreviewSwoop({ colors }: { colors: string[] }) {
    // Renders a small illustrative preview matching the "Lined" style — text-on-line
    // sweeps emanating from a central point. Falls back to the indigo theme palette
    // when the manifest entry doesn't supply colors.
    const palette =
        colors && colors.length > 0
            ? colors
            : ['#6366f1', '#8b5cf6', '#ec4899', '#f59e0b', '#10b981'];

    return (
        <svg viewBox="0 0 200 100" className="h-32 w-full">
            <line
                x1="80"
                y1="52"
                x2="120"
                y2="52"
                stroke="#94a3b8"
                strokeWidth="1.5"
                strokeLinecap="round"
            />
            {[
                { d: 'M 80 52 C 60 52, 50 24, 30 24', dy: 22 },
                { d: 'M 80 52 C 60 52, 50 78, 30 78', dy: 78 },
                { d: 'M 120 52 C 140 52, 150 18, 175 18', dy: 16 },
                { d: 'M 120 52 C 140 52, 150 50, 175 50', dy: 48 },
                { d: 'M 120 52 C 140 52, 150 84, 175 84', dy: 82 },
            ].map((p, i) => (
                <path
                    key={i}
                    d={p.d}
                    fill="none"
                    stroke={palette[i % palette.length]}
                    strokeWidth="2"
                    strokeLinecap="round"
                />
            ))}
        </svg>
    );
}

export function Templates() {
    const { t } = useLanguage();
    const [templates, setTemplates] = useState<TemplateEntry[]>([]);
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        fetch(`${TEMPLATES_BASE}manifest.json`)
            .then((r) => {
                if (!r.ok) throw new Error(`HTTP ${r.status}`);
                return r.json() as Promise<Manifest>;
            })
            .then((m) => setTemplates(m.templates ?? []))
            .catch((e) => setError(String(e)));
    }, []);

    return (
        <section id="templates" className="bg-white py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="mb-16 text-center">
                    <h2 className="mb-4 text-4xl font-bold text-slate-900">
                        {t('templates.title')}
                    </h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-600">
                        {t('templates.subtitle')}
                    </p>
                </div>

                <div className="mb-10 rounded-xl border border-slate-200 bg-slate-50 p-6">
                    <h3 className="mb-2 text-base font-semibold text-slate-900">
                        {t('templates.howto.title')}
                    </h3>
                    <p className="text-sm leading-relaxed text-slate-600">
                        {t('templates.howto.body')}
                    </p>
                    <ol className="mt-3 list-inside list-decimal space-y-1 text-sm text-slate-600">
                        <li>{t('templates.howto.step1')}</li>
                        <li>{t('templates.howto.step2')}</li>
                        <li>{t('templates.howto.step3')}</li>
                    </ol>
                </div>

                {error && (
                    <p className="mb-6 rounded-md border border-amber-200 bg-amber-50 p-3 text-sm text-amber-800">
                        {t('templates.loaderror')}: {error}
                    </p>
                )}

                <div className="grid gap-6 sm:grid-cols-2 lg:grid-cols-3">
                    {templates.map((tpl) => (
                        <div
                            key={tpl.id}
                            className="overflow-hidden rounded-xl border border-slate-200 bg-white shadow-sm transition-shadow hover:shadow-md"
                        >
                            <div className="bg-slate-50 px-4 py-3">
                                <PreviewSwoop colors={tpl.previewColors ?? []} />
                            </div>
                            <div className="p-5">
                                <h3 className="mb-1 text-lg font-semibold text-slate-900">
                                    {tpl.name}
                                </h3>
                                <p className="mb-4 text-sm leading-relaxed text-slate-600">
                                    {tpl.description}
                                </p>
                                <a
                                    href={`${TEMPLATES_BASE}${tpl.file}`}
                                    download={tpl.file}
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
                                    {t('templates.download')}
                                </a>
                            </div>
                        </div>
                    ))}
                </div>
            </div>
        </section>
    );
}
