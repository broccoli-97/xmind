export const zh: Record<string, string> = {
    // Navbar
    'nav.features': '功能',
    'nav.layouts': '布局',
    'nav.animation': '动画',
    'nav.themes': '主题',
    'nav.download': '下载',

    // Hero
    'hero.title': 'YMind',
    'hero.subtitle': '桌面思维导图编辑器',
    'hero.tagline': '精美的自动布局算法、流畅的动画效果，以及无干扰的编辑体验。开源且跨平台。',
    'hero.download': '立即下载',
    'hero.github': '在 GitHub 上查看',

    // Features
    'features.title': '强大功能',
    'features.subtitle': '满足您组织思维所需的一切，专为高性能打造。',

    'features.layouts.title': '三种布局样式',
    'features.layouts.desc': '双向布局、自上而下布局和右侧树形布局，适应您的思维方式。',

    'features.animation.title': '流畅动画',
    'features.animation.desc': '由三阶段算法管线驱动的自动布局，带来丝滑的过渡效果。',

    'features.themes.title': '暗色与亮色主题',
    'features.themes.desc': '在精心设计的暗色和亮色主题之间即时切换。',

    'features.export.title': '丰富的导出选项',
    'features.export.desc': '支持导出为 PNG、SVG、PDF、纯文本和 Markdown 格式。',

    'features.tabs.title': '标签页编辑',
    'features.tabs.desc': '通过标签页界面同时编辑多个思维导图。',

    'features.undo.title': '完整的撤销/重做',
    'features.undo.desc': '每个操作都可通过全面的撤销/重做支持来恢复。',

    'features.templates.title': '模板库',
    'features.templates.desc': '使用预设模板快速开始，覆盖常见使用场景。',

    'features.crossplatform.title': '跨平台',
    'features.crossplatform.desc':
        '在 Linux、Windows 和 macOS 上实现原生性能。使用 C++ 和 Qt6 构建。',

    // Layout Showcase
    'layouts.title': '自动布局算法',
    'layouts.subtitle': '三种独特的布局算法，每种都由三阶段管线驱动：测量、放置、优化。',
    'layouts.bilateral': '双向布局',
    'layouts.bilateral.desc':
        '节点在根节点两侧对称展开，非常适合头脑风暴和平衡概览。',
    'layouts.topdown': '自上而下',
    'layouts.topdown.desc':
        '从上到下的层次树结构，非常适合组织架构图和决策树。',
    'layouts.righttree': '右侧树',
    'layouts.righttree.desc': '经典的向右展开树形结构，适合大纲和顺序流程。',
    'layouts.phase.measure': '测量',
    'layouts.phase.measure.desc': '递归计算每个子树的边界框。',
    'layouts.phase.place': '放置',
    'layouts.phase.place.desc': '使用轴抽象的放置逻辑分配位置。',
    'layouts.phase.refine': '优化',
    'layouts.phase.refine.desc': '应用力导向优化以消除重叠。',

    // Animation Demo
    'animation.title': '流畅的自动布局',
    'animation.tagline': '当布局改变时，观察节点在平滑缓动曲线中滑入位置。',
    'animation.easing': '缓动曲线',
    'animation.easing.desc': '三次贝塞尔过渡确保每次节点重排都具有自然、令人满意的运动效果。',
    'animation.realtime': '实时计算',
    'animation.realtime.desc': '添加、移动或删除节点时布局即时重新计算——无需手动排列。',

    // Screenshots / Themes
    'themes.title': '精美主题',
    'themes.subtitle': '在为长时间编辑会话设计的亮色和暗色主题之间切换。',
    'themes.light': '亮色',
    'themes.dark': '暗色',

    // Download
    'download.title': '下载 YMind',
    'download.subtitle': '免费开源，支持所有主流平台。',
    'download.linux': 'Linux',
    'download.linux.desc': 'AppImage — 适用于大多数发行版。',
    'download.windows': 'Windows',
    'download.windows.desc': '便携 zip 包或 Microsoft Store。',
    'download.macos': 'macOS',
    'download.macos.desc': '适用于 Apple Silicon 的原生 ARM 应用。',
    'download.button': '下载最新版',
    'download.releases': '所有版本',

    // Footer
    'footer.license': '基于 Apache 2.0 许可证发布。',
    'footer.copyright': '© 2024-2026 YMind 贡献者。',
    'footer.source': '源代码',
};
