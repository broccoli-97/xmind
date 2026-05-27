#include "scene/NodeStyleResolver.h"
#include "core/TemplateDescriptor.h"

#include <QColor>
#include <QFont>
#include <QString>

NodeStyleResolver::NodeStyleResolver(const ThemeDescriptor* theme,
                                     const TemplateDescriptor* tpl)
    : m_theme(theme), m_template(tpl) {}

ThemeNodeStyle NodeStyleResolver::styleForLevel(int level) const {
    ThemeNodeStyle s = m_theme ? m_theme->nodeStyleForLevel(level) : ThemeNodeStyle{};
    if (m_template) {
        if (level == 0 && !m_template->rootShapeOverride.isEmpty())
            s.shape = m_template->rootShapeOverride;
        else if (!m_template->nodeShapeOverride.isEmpty())
            s.shape = m_template->nodeShapeOverride;
        if (!m_template->paletteSourceOverride.isEmpty())
            s.paletteSource = m_template->paletteSourceOverride;
    }
    return s;
}

QFont NodeStyleResolver::fontForLevel(int level, const QFont& base) const {
    const ThemeNodeStyle s = styleForLevel(level);
    QFont f = base;
    if (!s.fontFamily.isEmpty()) {
        f.setFamily(s.fontFamily);
        // Cursive style hint nudges Qt's font matcher toward a handwritten
        // fallback when the named family isn't installed — relevant for
        // custom themes that name a font we don't bundle.
        f.setStyleHint(QFont::Cursive, QFont::PreferDefault);
    }
    if (s.fontPointSize > 0.0)
        f.setPointSizeF(s.fontPointSize);
    return f;
}

bool NodeStyleResolver::drawsShadow(const ThemeNodeStyle& style, const QString& shape) {
    if (shape != QLatin1String("roundedRect"))
        return false;
    if (style.fillMode != QLatin1String("solid"))
        return false;
    return style.drawShadow;
}

QColor NodeStyleResolver::resolveBorderColor(const ThemeNodeStyle& style,
                                             const QColor& nodeColor,
                                             const QColor& fixedColor) {
    if (style.borderColorSource == QLatin1String("darker"))
        return nodeColor.darker(125);
    if (style.borderColorSource == QLatin1String("fixed") && fixedColor.isValid())
        return fixedColor;
    return nodeColor;
}
