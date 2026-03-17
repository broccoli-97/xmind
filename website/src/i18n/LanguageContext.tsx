import React, { createContext, useContext, useState, useCallback, useEffect } from 'react';
import { en } from './en';
import { zh } from './zh';

type Lang = 'en' | 'zh';

interface LanguageContextType {
    lang: Lang;
    setLang: (lang: Lang) => void;
    t: (key: string) => string;
}

const translations: Record<Lang, Record<string, string>> = { en, zh };

const LanguageContext = createContext<LanguageContextType | null>(null);

function detectLang(): Lang {
    const stored = localStorage.getItem('ymind-lang');
    if (stored === 'en' || stored === 'zh') return stored;
    return navigator.language.startsWith('zh') ? 'zh' : 'en';
}

export function LanguageProvider({ children }: { children: React.ReactNode }) {
    const [lang, setLangState] = useState<Lang>(detectLang);

    const setLang = useCallback((l: Lang) => {
        setLangState(l);
        localStorage.setItem('ymind-lang', l);
    }, []);

    useEffect(() => {
        document.documentElement.lang = lang;
    }, [lang]);

    const t = useCallback(
        (key: string): string => {
            return translations[lang][key] ?? key;
        },
        [lang]
    );

    return (
        <LanguageContext.Provider value={{ lang, setLang, t }}>{children}</LanguageContext.Provider>
    );
}

export function useLanguage(): LanguageContextType {
    const ctx = useContext(LanguageContext);
    if (!ctx) throw new Error('useLanguage must be used within LanguageProvider');
    return ctx;
}
