import { useLanguage } from '../i18n/LanguageContext';

export function AnimationDemo() {
    const { t } = useLanguage();
    const base = import.meta.env.BASE_URL;

    return (
        <section id="animation" className="bg-slate-50 py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="mb-16 text-center">
                    <h2 className="mb-4 text-4xl font-bold text-slate-900">
                        {t('animation.title')}
                    </h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-600">
                        {t('animation.tagline')}
                    </p>
                </div>

                <div className="grid items-center gap-12 lg:grid-cols-2">
                    {/* GIF in window frame */}
                    <div className="overflow-hidden rounded-xl border border-slate-200 bg-white shadow-lg">
                        <div className="flex h-8 items-center gap-2 border-b border-slate-200 bg-slate-100 px-4">
                            <span className="h-3 w-3 rounded-full bg-red-400" />
                            <span className="h-3 w-3 rounded-full bg-yellow-400" />
                            <span className="h-3 w-3 rounded-full bg-green-400" />
                        </div>
                        <img
                            src={`${base}img/AutoLayout.gif`}
                            alt="Auto-layout animation"
                            className="w-full"
                            loading="lazy"
                        />
                    </div>

                    {/* Description cards */}
                    <div className="space-y-6">
                        <div className="rounded-xl border border-slate-200 bg-white p-6">
                            <div className="mb-3 flex items-center gap-3">
                                <div className="flex h-10 w-10 items-center justify-center rounded-lg bg-violet-50 text-violet-600">
                                    <svg className="h-5 w-5" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={1.5}>
                                        <path strokeLinecap="round" strokeLinejoin="round" d="M3.75 13.5l10.5-11.25L12 10.5h8.25L9.75 21.75 12 13.5H3.75z" />
                                    </svg>
                                </div>
                                <h3 className="text-lg font-semibold text-slate-900">
                                    {t('animation.easing')}
                                </h3>
                            </div>
                            <p className="text-sm leading-relaxed text-slate-600">
                                {t('animation.easing.desc')}
                            </p>
                            {/* Easing curve visualization */}
                            <div className="mt-4 rounded-lg bg-slate-50 p-4">
                                <svg viewBox="0 0 200 100" className="h-20 w-full">
                                    <path d="M10 90 C 50 90, 60 10, 190 10" stroke="#7c3aed" strokeWidth="2.5" fill="none" />
                                    <line x1="10" y1="90" x2="190" y2="90" stroke="#e2e8f0" strokeWidth="1" strokeDasharray="4" />
                                    <line x1="10" y1="10" x2="190" y2="10" stroke="#e2e8f0" strokeWidth="1" strokeDasharray="4" />
                                    <circle cx="10" cy="90" r="3" fill="#7c3aed" />
                                    <circle cx="190" cy="10" r="3" fill="#7c3aed" />
                                </svg>
                            </div>
                        </div>

                        <div className="rounded-xl border border-slate-200 bg-white p-6">
                            <div className="mb-3 flex items-center gap-3">
                                <div className="flex h-10 w-10 items-center justify-center rounded-lg bg-indigo-50 text-indigo-600">
                                    <svg className="h-5 w-5" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={1.5}>
                                        <path strokeLinecap="round" strokeLinejoin="round" d="M12 6v6h4.5m4.5 0a9 9 0 11-18 0 9 9 0 0118 0z" />
                                    </svg>
                                </div>
                                <h3 className="text-lg font-semibold text-slate-900">
                                    {t('animation.realtime')}
                                </h3>
                            </div>
                            <p className="text-sm leading-relaxed text-slate-600">
                                {t('animation.realtime.desc')}
                            </p>
                        </div>
                    </div>
                </div>
            </div>
        </section>
    );
}
