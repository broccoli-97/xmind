import { useState } from 'react';
import { useLanguage } from '../i18n/LanguageContext';

type LayoutTab = 'bilateral' | 'topdown' | 'righttree';

const tabs: LayoutTab[] = ['bilateral', 'topdown', 'righttree'];

function BilateralSvg() {
    return (
        <svg viewBox="0 0 400 240" className="w-full" fill="none">
            {/* Root */}
            <rect x="165" y="100" width="70" height="36" rx="8" fill="#6366f1" />
            <text x="200" y="123" textAnchor="middle" fill="white" fontSize="13" fontWeight="600">Root</text>
            {/* Left children */}
            <rect x="30" y="40" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="70" y="60" textAnchor="middle" fill="white" fontSize="11">Topic A</text>
            <rect x="30" y="100" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="70" y="120" textAnchor="middle" fill="white" fontSize="11">Topic B</text>
            <rect x="30" y="160" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="70" y="180" textAnchor="middle" fill="white" fontSize="11">Topic C</text>
            {/* Right children */}
            <rect x="290" y="60" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="330" y="80" textAnchor="middle" fill="white" fontSize="11">Topic D</text>
            <rect x="290" y="140" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="330" y="160" textAnchor="middle" fill="white" fontSize="11">Topic E</text>
            {/* Edges */}
            <path d="M165 118 Q140 118 110 55" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M165 118 Q140 118 110 115" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M165 118 Q140 118 110 175" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M235 118 Q260 118 290 75" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M235 118 Q260 118 290 155" stroke="#a5b4fc" strokeWidth="2" fill="none" />
        </svg>
    );
}

function TopDownSvg() {
    return (
        <svg viewBox="0 0 400 260" className="w-full" fill="none">
            {/* Root */}
            <rect x="165" y="20" width="70" height="36" rx="8" fill="#6366f1" />
            <text x="200" y="43" textAnchor="middle" fill="white" fontSize="13" fontWeight="600">Root</text>
            {/* Level 1 */}
            <rect x="60" y="100" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="100" y="120" textAnchor="middle" fill="white" fontSize="11">Topic A</text>
            <rect x="260" y="100" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="300" y="120" textAnchor="middle" fill="white" fontSize="11">Topic B</text>
            {/* Level 2 */}
            <rect x="20" y="180" width="70" height="28" rx="6" fill="#a5b4fc" />
            <text x="55" y="198" textAnchor="middle" fill="#312e81" fontSize="10">Sub 1</text>
            <rect x="110" y="180" width="70" height="28" rx="6" fill="#a5b4fc" />
            <text x="145" y="198" textAnchor="middle" fill="#312e81" fontSize="10">Sub 2</text>
            <rect x="230" y="180" width="70" height="28" rx="6" fill="#a5b4fc" />
            <text x="265" y="198" textAnchor="middle" fill="#312e81" fontSize="10">Sub 3</text>
            <rect x="320" y="180" width="70" height="28" rx="6" fill="#a5b4fc" />
            <text x="355" y="198" textAnchor="middle" fill="#312e81" fontSize="10">Sub 4</text>
            {/* Edges */}
            <path d="M200 56 Q200 78 100 100" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M200 56 Q200 78 300 100" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M100 130 Q100 155 55 180" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M100 130 Q100 155 145 180" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M300 130 Q300 155 265 180" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M300 130 Q300 155 355 180" stroke="#a5b4fc" strokeWidth="2" fill="none" />
        </svg>
    );
}

function RightTreeSvg() {
    return (
        <svg viewBox="0 0 400 240" className="w-full" fill="none">
            {/* Root */}
            <rect x="20" y="100" width="70" height="36" rx="8" fill="#6366f1" />
            <text x="55" y="123" textAnchor="middle" fill="white" fontSize="13" fontWeight="600">Root</text>
            {/* Level 1 */}
            <rect x="140" y="40" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="180" y="60" textAnchor="middle" fill="white" fontSize="11">Topic A</text>
            <rect x="140" y="105" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="180" y="125" textAnchor="middle" fill="white" fontSize="11">Topic B</text>
            <rect x="140" y="170" width="80" height="30" rx="6" fill="#818cf8" />
            <text x="180" y="190" textAnchor="middle" fill="white" fontSize="11">Topic C</text>
            {/* Level 2 */}
            <rect x="280" y="20" width="70" height="26" rx="6" fill="#a5b4fc" />
            <text x="315" y="37" textAnchor="middle" fill="#312e81" fontSize="10">Sub 1</text>
            <rect x="280" y="60" width="70" height="26" rx="6" fill="#a5b4fc" />
            <text x="315" y="77" textAnchor="middle" fill="#312e81" fontSize="10">Sub 2</text>
            <rect x="280" y="150" width="70" height="26" rx="6" fill="#a5b4fc" />
            <text x="315" y="167" textAnchor="middle" fill="#312e81" fontSize="10">Sub 3</text>
            {/* Edges */}
            <path d="M90 118 Q115 118 140 55" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M90 118 Q115 118 140 120" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M90 118 Q115 118 140 185" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M220 55 Q250 55 280 33" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M220 55 Q250 55 280 73" stroke="#a5b4fc" strokeWidth="2" fill="none" />
            <path d="M220 185 Q250 185 280 163" stroke="#a5b4fc" strokeWidth="2" fill="none" />
        </svg>
    );
}

const layoutSvgs: Record<LayoutTab, React.ReactNode> = {
    bilateral: <BilateralSvg />,
    topdown: <TopDownSvg />,
    righttree: <RightTreeSvg />,
};

export function LayoutShowcase() {
    const { t } = useLanguage();
    const [active, setActive] = useState<LayoutTab>('bilateral');

    return (
        <section id="layouts" className="bg-slate-900 py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="mb-16 text-center">
                    <h2 className="mb-4 text-4xl font-bold text-white">{t('layouts.title')}</h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-400">
                        {t('layouts.subtitle')}
                    </p>
                </div>

                {/* Tabs */}
                <div className="mb-8 flex justify-center gap-2">
                    {tabs.map((tab) => (
                        <button
                            key={tab}
                            onClick={() => setActive(tab)}
                            className={`rounded-lg px-5 py-2.5 text-sm font-medium transition-colors ${
                                active === tab
                                    ? 'bg-indigo-500 text-white'
                                    : 'bg-slate-800 text-slate-400 hover:bg-slate-700 hover:text-slate-200'
                            }`}
                        >
                            {t(`layouts.${tab}`)}
                        </button>
                    ))}
                </div>

                {/* SVG + Description */}
                <div className="grid items-center gap-8 lg:grid-cols-2">
                    <div className="overflow-hidden rounded-xl border border-slate-700 bg-slate-800/50 p-6">
                        {layoutSvgs[active]}
                    </div>
                    <div>
                        <p className="mb-8 text-lg leading-relaxed text-slate-300">
                            {t(`layouts.${active}.desc`)}
                        </p>

                        {/* 3-Phase Pipeline */}
                        <div className="space-y-4">
                            {(['measure', 'place', 'refine'] as const).map((phase, i) => (
                                <div key={phase} className="flex items-start gap-4">
                                    <div className="flex h-8 w-8 flex-shrink-0 items-center justify-center rounded-full bg-indigo-500/20 text-sm font-bold text-indigo-400">
                                        {i + 1}
                                    </div>
                                    <div>
                                        <h4 className="font-semibold text-white">
                                            {t(`layouts.phase.${phase}`)}
                                        </h4>
                                        <p className="text-sm text-slate-400">
                                            {t(`layouts.phase.${phase}.desc`)}
                                        </p>
                                    </div>
                                </div>
                            ))}
                        </div>
                    </div>
                </div>
            </div>
        </section>
    );
}
