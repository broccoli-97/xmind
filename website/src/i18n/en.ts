export const en: Record<string, string> = {
    // Navbar
    'nav.features': 'Features',
    'nav.layouts': 'Layouts',
    'nav.animation': 'Animation',
    'nav.themes': 'Themes',
    'nav.mcp': 'AI / MCP',
    'nav.download': 'Download',

    // Hero
    'hero.title': 'YMind',
    'hero.subtitle': 'Desktop Mind Map Editor',
    'hero.tagline':
        'Beautiful auto-layout algorithms, smooth animations, and a distraction-free editing experience. Open source & cross-platform.',
    'hero.download': 'Download',
    'hero.github': 'View on GitHub',

    // Features
    'features.title': 'Powerful Features',
    'features.subtitle': 'Everything you need to organize your thoughts, built with performance in mind.',

    'features.layouts.title': 'Three Layout Styles',
    'features.layouts.desc':
        'Bilateral, Top-Down, and Right-Tree layouts adapt to your thinking style.',

    'features.animation.title': 'Smooth Animations',
    'features.animation.desc':
        'Auto-layout with fluid transitions powered by a 3-phase algorithm pipeline.',

    'features.themes.title': 'Dark & Light Themes',
    'features.themes.desc': 'Switch between beautifully crafted dark and light themes instantly.',

    'features.export.title': 'Rich Export Options',
    'features.export.desc': 'Export to PNG, SVG, PDF, plain text, and Markdown formats.',

    'features.tabs.title': 'Tabbed Editing',
    'features.tabs.desc': 'Work on multiple mind maps simultaneously with tabbed interface.',

    'features.undo.title': 'Full Undo/Redo',
    'features.undo.desc': 'Every action is reversible with comprehensive undo/redo support.',

    'features.templates.title': 'Templates Gallery',
    'features.templates.desc': 'Start quickly with pre-designed templates for common use cases.',

    'features.crossplatform.title': 'Cross-Platform',
    'features.crossplatform.desc':
        'Native performance on Linux, Windows, and macOS. Built with C++ and Qt6.',

    // Layout Showcase
    'layouts.title': 'Auto-Layout Algorithms',
    'layouts.subtitle':
        'Three distinct layout algorithms, each powered by a 3-phase pipeline: Measure, Place, Refine.',
    'layouts.bilateral': 'Bilateral',
    'layouts.bilateral.desc':
        'Nodes spread symmetrically on both sides of the root, ideal for brainstorming and balanced overviews.',
    'layouts.topdown': 'Top-Down',
    'layouts.topdown.desc':
        'Hierarchical tree flowing from top to bottom, perfect for org charts and decision trees.',
    'layouts.righttree': 'Right Tree',
    'layouts.righttree.desc':
        'Classic right-expanding tree, great for outlines and sequential processes.',
    'layouts.phase.measure': 'Measure',
    'layouts.phase.measure.desc': 'Calculate the bounding box of every subtree recursively.',
    'layouts.phase.place': 'Place',
    'layouts.phase.place.desc': 'Assign positions using axis-abstracted placement logic.',
    'layouts.phase.refine': 'Refine',
    'layouts.phase.refine.desc': 'Apply force-directed refinement to eliminate overlaps.',

    // Animation Demo
    'animation.title': 'Fluid Auto-Layout',
    'animation.tagline':
        'Watch nodes glide into position with smooth easing curves when the layout changes.',
    'animation.easing': 'Easing Curve',
    'animation.easing.desc':
        'Cubic-bezier transitions ensure natural, satisfying motion for every node rearrangement.',
    'animation.realtime': 'Real-Time',
    'animation.realtime.desc':
        'Layout recalculates instantly as you add, move, or delete nodes — no manual arrangement needed.',

    // Screenshots (light/dark showcase strip)
    'screenshots.title': 'Beautiful Themes',
    'screenshots.subtitle':
        'Switch between light and dark themes designed for extended editing sessions.',
    'screenshots.light': 'Light',
    'screenshots.dark': 'Dark',

    // Download
    'download.title': 'Download YMind',
    'download.subtitle': 'Free and open source. Available on all major platforms.',
    'download.linux': 'Linux',
    'download.linux.desc': 'AppImage — works on most distributions.',
    'download.windows': 'Windows',
    'download.windows.desc': 'Portable zip or Microsoft Store.',
    'download.macos': 'macOS',
    'download.macos.desc': 'Native ARM app bundle for Apple Silicon.',
    'download.button': 'Download Latest',
    'download.releases': 'All Releases',

    // Themes
    'themes.title': 'Themes',
    'themes.subtitle':
        'Drop-in visual styles that change the look of your mind maps — colors, fills, borders, edges. The app already ships a curated set; download more here.',
    'themes.howto.title': 'How to install',
    'themes.howto.body':
        'Each theme is a small folder containing theme.json (and optional preview). Drop the whole folder into your YMind templates directory; the app picks it up on next launch.',
    'themes.howto.step1':
        'Download a theme.json from a card below (or grab the whole folder).',
    'themes.howto.step2':
        'Place it into your YMind templates folder, either as <id>/theme.json or as a flat <id>.json. Linux/macOS: ~/.local/share/YMind/templates/. Windows: %APPDATA%/YMind/templates/.',
    'themes.howto.step3':
        'Restart YMind, then pick the new theme under Theme → Switch Theme.',
    'themes.download': 'Download',
    'themes.loaderror': 'Could not load themes manifest',

    // MCP / AI
    'mcp.badge': 'Model Context Protocol',
    'mcp.title': 'Use YMind from Claude, Cursor, or any AI',
    'mcp.subtitle':
        'YMind ships an MCP server that lets an AI turn Markdown into a beautiful auto-laid-out mindmap — runs locally, no cloud account needed.',

    'mcp.tool.render.desc':
        'Convert a Markdown outline into an SVG or PNG mindmap with a chosen layout and theme.',
    'mcp.tool.layouts.desc':
        'List the supported layout algorithms (bilateral, top-down, right-tree).',
    'mcp.tool.themes.desc':
        'List the available themes so the AI can pick one that matches the topic.',

    'mcp.install.recommendedTag': 'Recommended',
    'mcp.install.pluginTitle': 'Install as a Claude Code plugin',
    'mcp.install.pluginBody':
        'YMind ships as a Claude Code plugin. Two commands — no JSON editing, works in any session.',
    'mcp.install.pluginNote':
        'Run /reload-plugins (or restart Claude Code) after install. The MCP server registers automatically once enabled.',

    'mcp.path.title': 'One-time setup: put ymind-cli on PATH',
    'mcp.path.body':
        'The plugin invokes the ymind-cli binary by name — it has to be discoverable in your shell. Symlink it once and you are done.',
    'mcp.path.linuxMac': 'Linux / macOS',
    'mcp.path.fromSource': 'From source build',

    'mcp.manual.title': 'Other clients (Claude Desktop, Cursor)',
    'mcp.manual.body':
        'These clients do not support the Claude Code plugin marketplace yet. Drop this JSON into their MCP config:',
    'mcp.client.claudeDesktop': 'Claude Desktop',
    'mcp.client.cursor': 'Cursor',

    'mcp.example.title': 'Try it',
    'mcp.example.body':
        'After registering the server, ask any MCP-enabled assistant something like:',
    'mcp.example.youSay': 'You',
    'mcp.example.prompt':
        'Summarise this article as a mindmap. Use the bilateral layout and the Nord theme, then save the PNG to my desktop.',

    'mcp.build.label': 'No installed binary?',
    'mcp.build.body':
        'Clone the repo and build from source — ymind-cli is produced alongside the GUI app:',

    // Footer
    'footer.license': 'Released under the Apache 2.0 License.',
    'footer.copyright': '© 2024-2026 YMind Contributors.',
    'footer.source': 'Source Code',
};
