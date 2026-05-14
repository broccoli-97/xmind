export const zh: Record<string, string> = {
    // Navbar
    'nav.features': '功能',
    'nav.layouts': '布局',
    'nav.animation': '动画',
    'nav.themes': '主题',
    'nav.mcp': 'AI / MCP',
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

    // Screenshots (light/dark showcase strip)
    'screenshots.title': '精美主题',
    'screenshots.subtitle': '在为长时间编辑会话设计的亮色和暗色主题之间切换。',
    'screenshots.light': '亮色',
    'screenshots.dark': '暗色',

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

    // Themes
    'themes.title': '主题',
    'themes.subtitle':
        '可直接套用的视觉风格，改变思维导图的颜色、填充、边框与连接线。应用已内置多种主题，您也可以在此下载更多。',
    'themes.howto.title': '安装方式',
    'themes.howto.body':
        '每个主题都是一个小文件夹，包含 theme.json 与可选预览图。把整个文件夹放进 YMind 的模板目录，下次启动即可使用。',
    'themes.howto.step1': '从下方任意卡片下载 theme.json（或整个主题文件夹）。',
    'themes.howto.step2':
        '把文件放入 YMind 模板目录，可作为 <id>/theme.json 子文件夹，或直接作为 <id>.json 平铺。Linux/macOS：~/.local/share/YMind/templates/，Windows：%APPDATA%/YMind/templates/。',
    'themes.howto.step3': '重启 YMind，然后在「主题 → 切换主题」中选择新主题。',
    'themes.download': '下载',
    'themes.loaderror': '无法加载主题清单',

    // MCP / AI
    'mcp.badge': 'Model Context Protocol',
    'mcp.title': '在 Claude、Cursor 等 AI 中使用 YMind',
    'mcp.subtitle':
        'YMind 内置了一个 MCP 服务器，可以让 AI 把 Markdown 自动排版为精美的思维导图。完全在本地运行，无需任何云端账号。',

    'mcp.tool.render.desc':
        '把 Markdown 大纲转换为 SVG 或 PNG 思维导图，可指定布局与主题。',
    'mcp.tool.layouts.desc':
        '列出支持的布局算法（双向、自上而下、右侧树）。',
    'mcp.tool.themes.desc':
        '列出可用主题，方便 AI 根据内容挑选合适的视觉风格。',

    'mcp.install.recommendedTag': '推荐',
    'mcp.install.pluginTitle': '作为 Claude Code 插件安装',
    'mcp.install.pluginBody':
        'YMind 已发布为 Claude Code 插件。两条命令搞定，无需手写 JSON，在任意会话中均可使用。',
    'mcp.install.pluginNote':
        '安装后运行 /reload-plugins（或重启 Claude Code）。插件启用后 MCP 服务器会自动注册。',

    'mcp.path.title': '一次性准备：把 ymind-cli 加到 PATH',
    'mcp.path.body':
        '插件通过 ymind-cli 这个命令名调用二进制，所以它必须能在 shell 中被找到。建立一次软链接即可永久解决。',
    'mcp.path.linuxMac': 'Linux / macOS',
    'mcp.path.fromSource': '源码构建',

    'mcp.manual.title': '其他客户端（Claude 桌面版、Cursor）',
    'mcp.manual.body':
        '这些客户端暂时还不支持 Claude Code 插件市场。把下面的 JSON 粘贴到它们的 MCP 配置即可：',
    'mcp.client.claudeDesktop': 'Claude 桌面版',
    'mcp.client.cursor': 'Cursor',

    'mcp.example.title': '试一试',
    'mcp.example.body': '注册成功后，对支持 MCP 的助手说类似这样的话：',
    'mcp.example.youSay': '你',
    'mcp.example.prompt':
        '把这篇文章总结成思维导图，使用双向布局和 Nord 主题，然后把 PNG 保存到桌面。',

    'mcp.build.label': '没有现成的安装包？',
    'mcp.build.body':
        '克隆仓库后直接从源码构建即可，ymind-cli 会和 GUI 主程序一起生成：',

    // Footer
    'footer.license': '基于 Apache 2.0 许可证发布。',
    'footer.copyright': '© 2024-2026 YMind 贡献者。',
    'footer.source': '源代码',
};
