import { useLanguage } from '../i18n/LanguageContext';

export function Hero() {
    const { t } = useLanguage();
    const base = import.meta.env.BASE_URL;

    return (
        <section className="relative flex min-h-screen items-center justify-center overflow-hidden bg-gradient-to-b from-indigo-950 via-violet-900 to-slate-900 pt-16">
            {/* Animated background circles */}
            <div className="pointer-events-none absolute inset-0 overflow-hidden">
                <div className="absolute -left-40 -top-40 h-[500px] w-[500px] rounded-full bg-indigo-500/10 blur-3xl" />
                <div className="absolute -bottom-40 -right-40 h-[600px] w-[600px] rounded-full bg-violet-500/10 blur-3xl" />
                <div className="absolute left-1/2 top-1/3 h-[300px] w-[300px] -translate-x-1/2 rounded-full bg-purple-500/10 blur-3xl" />
            </div>

            <div className="relative z-10 mx-auto flex max-w-6xl flex-col items-center px-4 py-20 text-center sm:px-6">
                <h1 className="mb-2 text-6xl font-extrabold tracking-tight text-white sm:text-7xl">
                    {t('hero.title')}
                </h1>
                <p className="mb-4 text-xl font-medium text-indigo-200 sm:text-2xl">
                    {t('hero.subtitle')}
                </p>
                <p className="mb-8 max-w-2xl text-lg text-slate-300">{t('hero.tagline')}</p>

                <div className="mb-12 flex flex-wrap justify-center gap-4">
                    <a
                        href="#download"
                        className="rounded-lg bg-indigo-500 px-6 py-3 font-semibold text-white shadow-lg shadow-indigo-500/25 transition-all hover:bg-indigo-400 hover:shadow-indigo-500/40"
                    >
                        {t('hero.download')}
                    </a>
                    <a
                        href="https://github.com/broccoli-97/xmind"
                        target="_blank"
                        rel="noopener noreferrer"
                        className="rounded-lg border border-white/20 px-6 py-3 font-semibold text-white transition-all hover:border-white/40 hover:bg-white/5"
                    >
                        {t('hero.github')}
                    </a>
                </div>

                {/* Screenshot mockup */}
                <div className="w-full max-w-4xl">
                    <div className="overflow-hidden rounded-xl border border-white/10 bg-slate-800/50 shadow-2xl shadow-black/40">
                        <div className="flex h-8 items-center gap-2 border-b border-white/10 bg-slate-800/80 px-4">
                            <span className="h-3 w-3 rounded-full bg-red-400/80" />
                            <span className="h-3 w-3 rounded-full bg-yellow-400/80" />
                            <span className="h-3 w-3 rounded-full bg-green-400/80" />
                        </div>
                        <img
                            src={`${base}img/Screenshot.png`}
                            alt="YMind Screenshot"
                            className="w-full"
                            loading="eager"
                        />
                    </div>
                </div>
            </div>
        </section>
    );
}
