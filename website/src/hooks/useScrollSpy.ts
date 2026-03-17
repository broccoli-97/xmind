import { useEffect, useState } from 'react';

export function useScrollSpy(ids: string[], offset = 80): string {
    const [activeId, setActiveId] = useState('');

    useEffect(() => {
        const observer = new IntersectionObserver(
            (entries) => {
                for (const entry of entries) {
                    if (entry.isIntersecting) {
                        setActiveId(entry.target.id);
                    }
                }
            },
            { rootMargin: `-${offset}px 0px -60% 0px`, threshold: 0 }
        );

        for (const id of ids) {
            const el = document.getElementById(id);
            if (el) observer.observe(el);
        }

        return () => observer.disconnect();
    }, [ids, offset]);

    return activeId;
}
