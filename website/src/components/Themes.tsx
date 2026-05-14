import { useEffect, useState } from 'react';
import { useLanguage } from '../i18n/LanguageContext';

type Localized = string | { en: string; zh?: string };

type ThemeEntry = {
    id: string;
    name: Localized;
    description: Localized;
    folder: string;
};

type Manifest = {
    themes: ThemeEntry[];
};

// Subset of the full theme.json we care about for previews.
type ThemeData = {
    nodeStyle?: {
        borderRadius?: number;
        fillAlpha?: number;
        borderWidth?: number;
        fillMode?: string;
    };
    edgeStyle?: {
        width?: number;
        curvature?: number;
    };
    colors?: {
        light?: {
            canvasBackground?: string;
            canvasGridDot?: string;
            nodePalette?: string[];
            nodeText?: string;
            nodeSelectionBorder?: string;
        };
    };
};

const THEMES_BASE = `${import.meta.env.BASE_URL}themes/`;

function pickLocalized(s: Localized, lang: 'en' | 'zh'): string {
    if (typeof s === 'string') return s;
    return s[lang] ?? s.en ?? '';
}

// Visual preview rendered from the actual theme.json — uses each theme's own
// borderRadius / fillAlpha / borderWidth / curvature so previews differ
// structurally, not only by palette.
function MiniMap({ data }: { data: ThemeData }) {
    const light = data.colors?.light ?? {};
    const palette =
        light.nodePalette && light.nodePalette.length > 0
            ? light.nodePalette
            : ['#6366f1', '#8b5cf6', '#ec4899', '#f59e0b', '#10b981', '#06b6d4'];
    const bg = light.canvasBackground ?? '#FFFFFF';
    const gridDot = light.canvasGridDot ?? '#E5E5E5';
    const textColor = light.nodeText ?? '#1F2937';

    const ns = data.nodeStyle ?? {};
    const radius = (ns.borderRadius ?? 8) * 0.6; // mini-map scale
    const fillAlpha = ns.fillAlpha ?? 0.6;
    const fillMode = ns.fillMode ?? 'tinted';
    const borderWidth = ns.borderWidth ?? 1.4;
    const isOutlined = fillMode === 'outlined';
    const isSolid = fillMode === 'solid';

    const es = data.edgeStyle ?? {};
    const edgeWidth = es.width ?? 1.6;
    const curvature = es.curvature ?? 0.55;

    // Mind-map layout: central root + 5 surrounding branches with varied sizes
    // so the preview reads as a real map, not a coloring grid.
    const root = { x: 100, y: 50, w: 50, h: 22 };
    const branches = [
        { x: 30, y: 18, w: 36, h: 16, paletteIdx: 1 },
        { x: 30, y: 50, w: 40, h: 16, paletteIdx: 2 },
        { x: 30, y: 82, w: 32, h: 14, paletteIdx: 3 },
        { x: 170, y: 25, w: 36, h: 16, paletteIdx: 4 },
        { x: 170, y: 60, w: 42, h: 16, paletteIdx: 5 },
    ];

    function bezierTo(b: { x: number; y: number; w: number; h: number }) {
        const isLeft = b.x < root.x;
        const sx = isLeft ? root.x - root.w / 2 : root.x + root.w / 2;
        const sy = root.y;
        const ex = isLeft ? b.x + b.w / 2 : b.x - b.w / 2;
        const ey = b.y;
        const dx = ex - sx;
        const cx1 = sx + dx * curvature;
        const cx2 = ex - dx * curvature;
        return `M ${sx} ${sy} C ${cx1} ${sy}, ${cx2} ${ey}, ${ex} ${ey}`;
    }

    function nodeFill(color: string, isRoot: boolean) {
        if (isOutlined) return 'transparent';
        const alpha = isRoot ? Math.min(1, fillAlpha + 0.18) : fillAlpha;
        return isSolid ? color : color;
        // We use fillOpacity below to apply the alpha for tinted/solid.
        void alpha;
    }

    function nodeFillOpacity(isRoot: boolean) {
        if (isOutlined) return 0;
        if (isSolid) return 1;
        return isRoot ? Math.min(1, fillAlpha + 0.18) : fillAlpha;
    }

    // Subtle dot pattern using a tiny lattice (5px grid) so users can spot
    // canvas color + dot tone differences between themes.
    const dots: { x: number; y: number }[] = [];
    for (let y = 8; y < 100; y += 12) {
        for (let x = 8; x < 200; x += 12) dots.push({ x, y });
    }

    return (
        <svg viewBox="0 0 200 100" className="h-32 w-full" style={{ background: bg }}>
            {dots.map((d, i) => (
                <circle key={`d${i}`} cx={d.x} cy={d.y} r={0.7} fill={gridDot} />
            ))}

            {branches.map((b, i) => {
                const c = palette[b.paletteIdx % palette.length] ?? palette[0] ?? '#888';
                return (
                    <path
                        key={`e${i}`}
                        d={bezierTo(b)}
                        fill="none"
                        stroke={c}
                        strokeWidth={edgeWidth}
                        strokeLinecap="round"
                    />
                );
            })}

            <rect
                x={root.x - root.w / 2}
                y={root.y - root.h / 2}
                width={root.w}
                height={root.h}
                rx={radius}
                fill={nodeFill(palette[0] ?? '#888', true)}
                fillOpacity={nodeFillOpacity(true)}
                stroke={palette[0] ?? '#888'}
                strokeWidth={borderWidth}
            />

            {branches.map((b, i) => {
                const c = palette[b.paletteIdx % palette.length] ?? palette[0] ?? '#888';
                return (
                    <rect
                        key={`n${i}`}
                        x={b.x - b.w / 2}
                        y={b.y - b.h / 2}
                        width={b.w}
                        height={b.h}
                        rx={radius}
                        fill={nodeFill(c, false)}
                        fillOpacity={nodeFillOpacity(false)}
                        stroke={c}
                        strokeWidth={borderWidth}
                    />
                );
            })}

            <text
                x={root.x}
                y={root.y + 3.5}
                fontSize="8"
                fontWeight="600"
                textAnchor="middle"
                fill={textColor}
                fontFamily="system-ui, sans-serif"
            >
                YMind
            </text>
        </svg>
    );
}

export function Themes() {
    const { t, lang } = useLanguage();
    const [themes, setThemes] = useState<ThemeEntry[]>([]);
    const [themeData, setThemeData] = useState<Record<string, ThemeData>>({});
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        fetch(`${THEMES_BASE}manifest.json`)
            .then((r) => {
                if (!r.ok) throw new Error(`HTTP ${r.status}`);
                return r.json() as Promise<Manifest>;
            })
            .then((m) => {
                const list = m.themes ?? [];
                setThemes(list);
                // Fetch each theme.json in parallel so previews can render
                // with the real per-theme style attributes.
                Promise.all(
                    list.map((th) =>
                        fetch(`${THEMES_BASE}${th.folder}/theme.json`)
                            .then((r) => (r.ok ? r.json() : null))
                            .then((d) => [th.id, d as ThemeData | null] as const)
                            .catch(() => [th.id, null] as const)
                    )
                ).then((entries) => {
                    const map: Record<string, ThemeData> = {};
                    for (const [id, d] of entries) if (d) map[id] = d;
                    setThemeData(map);
                });
            })
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
                                <MiniMap data={themeData[th.id] ?? {}} />
                            </div>
                            <div className="p-5">
                                <h3 className="mb-1 text-lg font-semibold text-slate-900">
                                    {pickLocalized(th.name, lang)}
                                </h3>
                                <p className="mb-4 text-sm leading-relaxed text-slate-600">
                                    {pickLocalized(th.description, lang)}
                                </p>
                                <a
                                    href={`${THEMES_BASE}${th.folder}/theme.json`}
                                    download={`${th.folder}.json`}
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
