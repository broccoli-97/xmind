import { useLanguage } from '../i18n/LanguageContext';

const platforms = [
    {
        key: 'linux',
        icon: (
            <img
                src={`${import.meta.env.BASE_URL}icons/linux.svg`}
                alt="Linux"
                className="h-10 w-10"
                loading="lazy"
            />
        ),
    },
    {
        key: 'windows',
        icon: (
            <svg className="h-10 w-10" viewBox="0 0 24 24" fill="currentColor">
                <path d="M0 3.449L9.75 2.1v9.451H0m10.949-9.602L24 0v11.4H10.949M0 12.6h9.75v9.451L0 20.699M10.949 12.6H24V24l-12.9-1.801" />
            </svg>
        ),
    },
    {
        key: 'macos',
        icon: (
            <svg className="h-10 w-10" viewBox="0 0 24 24" fill="currentColor">
                <path d="M18.71 19.5c-.83 1.24-1.71 2.45-3.05 2.47-1.34.03-1.77-.79-3.29-.79-1.53 0-2 .77-3.27.82-1.31.05-2.3-1.32-3.14-2.53C4.25 17 2.94 12.45 4.7 9.39c.87-1.52 2.43-2.48 4.12-2.51 1.28-.02 2.5.87 3.29.87.78 0 2.26-1.07 3.8-.91.65.03 2.47.26 3.64 1.98-.09.06-2.17 1.28-2.15 3.81.03 3.02 2.65 4.03 2.68 4.04-.03.07-.42 1.44-1.38 2.83M13 3.5c.73-.83 1.94-1.46 2.94-1.5.13 1.17-.34 2.35-1.04 3.19-.69.85-1.83 1.51-2.95 1.42-.15-1.15.41-2.35 1.05-3.11z" />
            </svg>
        ),
    },
];

const RELEASES_URL = 'https://github.com/broccoli-97/xmind/releases';

export function Download() {
    const { t } = useLanguage();

    return (
        <section id="download" className="bg-gradient-to-b from-slate-50 to-indigo-50 py-24">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="mb-16 text-center">
                    <h2 className="mb-4 text-4xl font-bold text-slate-900">
                        {t('download.title')}
                    </h2>
                    <p className="mx-auto max-w-2xl text-lg text-slate-600">
                        {t('download.subtitle')}
                    </p>
                </div>

                <div className="grid gap-6 sm:grid-cols-3">
                    {platforms.map((p) => (
                        <a
                            key={p.key}
                            href={`${RELEASES_URL}/latest`}
                            target="_blank"
                            rel="noopener noreferrer"
                            className="group rounded-xl border border-slate-200 bg-white p-8 text-center shadow-sm transition-all hover:border-indigo-300 hover:shadow-md"
                        >
                            <div className="mb-4 flex justify-center text-slate-700 transition-colors group-hover:text-indigo-600">
                                {p.icon}
                            </div>
                            <h3 className="mb-2 text-xl font-semibold text-slate-900">
                                {t(`download.${p.key}`)}
                            </h3>
                            <p className="mb-6 text-sm text-slate-500">
                                {t(`download.${p.key}.desc`)}
                            </p>
                            <span className="inline-block rounded-lg bg-indigo-500 px-5 py-2.5 text-sm font-medium text-white transition-colors group-hover:bg-indigo-400">
                                {t('download.button')}
                            </span>
                        </a>
                    ))}
                </div>

                <div className="mt-8 text-center">
                    <a
                        href={RELEASES_URL}
                        target="_blank"
                        rel="noopener noreferrer"
                        className="text-sm font-medium text-indigo-600 hover:text-indigo-500"
                    >
                        {t('download.releases')} &rarr;
                    </a>
                </div>
            </div>
        </section>
    );
}
