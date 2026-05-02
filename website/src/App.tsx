import { LanguageProvider } from './i18n/LanguageContext';
import { Navbar } from './components/Navbar';
import { Hero } from './components/Hero';
import { Features } from './components/Features';
import { LayoutShowcase } from './components/LayoutShowcase';
import { AnimationDemo } from './components/AnimationDemo';
import { Screenshots } from './components/Screenshots';
import { Templates } from './components/Templates';
import { Download } from './components/Download';
import { Footer } from './components/Footer';

export default function App() {
    return (
        <LanguageProvider>
            <Navbar />
            <main>
                <Hero />
                <Features />
                <LayoutShowcase />
                <AnimationDemo />
                <Screenshots />
                <Templates />
                <Download />
            </main>
            <Footer />
        </LanguageProvider>
    );
}
