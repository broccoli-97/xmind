#pragma once

#include "core/TemplateDescriptor.h"

#include <QCoreApplication>
#include <QList>
#include <QMap>
#include <QString>

class TemplateRegistry {
    Q_DECLARE_TR_FUNCTIONS(TemplateRegistry)
public:
    static TemplateRegistry& instance();

    void loadBuiltins();
    void loadFromDirectory(const QString& dirPath);

    void registerTemplate(const TemplateDescriptor& td);

    const TemplateDescriptor* templateById(const QString& id) const;
    QList<const TemplateDescriptor*> allTemplates() const;

    // Map old layout style int to builtin template ID
    static QString builtinIdForLayoutStyle(int layoutStyleInt);

private:
    TemplateRegistry() = default;
    QMap<QString, TemplateDescriptor> m_templates;
    QList<QString> m_orderedIds; // insertion order
};
