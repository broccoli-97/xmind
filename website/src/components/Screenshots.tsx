import { useState } from 'react';
import { useLanguage } from '../i18n/LanguageContext';

export function Screenshots() {
    const { t } = useLanguage();
    const base = import.meta.env.BASE_URL;
    const [theme, setTheme] = useState<'light' | 'dark'>('light');

    return (
        <section id="themes" className="bg-slate-900 py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="mb-16 text-center">
                    <h2 className="mb-4 text-4xl font-bold text-white">{t('themes.title')}</h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-400">
                        {t('themes.subtitle')}
                    </p>
                </div>

                {/* Toggle */}
                <div className="mb-8 flex justify-center">
                    <div className="inline-flex rounded-lg bg-slate-800 p-1">
                        <button
                            onClick={() => setTheme('light')}
                            className={`rounded-md px-5 py-2 text-sm font-medium transition-colors ${
                                theme === 'light'
                                    ? 'bg-white text-slate-900 shadow'
                                    : 'text-slate-400 hover:text-slate-200'
                            }`}
                        >
                            {t('themes.light')}
                        </button>
                        <button
                            onClick={() => setTheme('dark')}
                            className={`rounded-md px-5 py-2 text-sm font-medium transition-colors ${
                                theme === 'dark'
                                    ? 'bg-slate-600 text-white shadow'
                                    : 'text-slate-400 hover:text-slate-200'
                            }`}
                        >
                            {t('themes.dark')}
                        </button>
                    </div>
                </div>

                {/* Screenshot */}
                <div className="mx-auto max-w-4xl overflow-hidden rounded-xl border border-slate-700 shadow-2xl shadow-black/40">
                    <div className="flex h-8 items-center gap-2 border-b border-slate-700 bg-slate-800 px-4">
                        <span className="h-3 w-3 rounded-full bg-red-400/80" />
                        <span className="h-3 w-3 rounded-full bg-yellow-400/80" />
                        <span className="h-3 w-3 rounded-full bg-green-400/80" />
                    </div>
                    <img
                        src={`${base}img/${theme === 'light' ? 'LightTheme' : 'DarkTheme'}.png`}
                        alt={`${theme} theme`}
                        className="w-full"
                        loading="lazy"
                    />
                </div>
            </div>
        </section>
    );
}
