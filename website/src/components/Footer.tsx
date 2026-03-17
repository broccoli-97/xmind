import { useLanguage } from '../i18n/LanguageContext';

export function Footer() {
    const { t } = useLanguage();

    return (
        <footer className="bg-slate-950 py-12">
            <div className="mx-auto max-w-6xl px-4 sm:px-6">
                <div className="flex flex-col items-center gap-4 text-center">
                    <a
                        href="https://github.com/broccoli-97/xmind"
                        target="_blank"
                        rel="noopener noreferrer"
                        className="text-sm font-medium text-slate-400 transition-colors hover:text-white"
                    >
                        {t('footer.source')}
                    </a>
                    <p className="text-sm text-slate-500">{t('footer.license')}</p>
                    <p className="text-sm text-slate-500">{t('footer.copyright')}</p>
                </div>
            </div>
        </footer>
    );
}
