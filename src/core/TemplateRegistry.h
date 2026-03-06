#pragma once

#include "core/TemplateDescriptor.h"

#include <QList>
#include <QMap>
#include <QString>

class TemplateRegistry {
public:
    static TemplateRegistry& instance();

    void loadBuiltins();
    void loadFromDirectory(const QString& dirPath);

    const TemplateDescriptor* templateById(const QString& id) const;
    QList<const TemplateDescriptor*> allTemplates() const;

    // Map old layout style int to builtin template ID
    static QString builtinIdForLayoutStyle(int layoutStyleInt);

private:
    TemplateRegistry() = default;
    QMap<QString, TemplateDescriptor> m_templates;
    QList<QString> m_orderedIds; // insertion order
};
