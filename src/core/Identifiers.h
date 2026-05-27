#pragma once

#include <QHash>
#include <QString>
#include <utility>

// Strong typedefs for template and theme identifiers. Wrapping a QString stops
// the two from being silently mixed up in function signatures, and turns the
// scattered `if (!id.isEmpty())` defensive checks at the boundary of every
// scene/serializer/menu callsite into a single `id.isValid()`.
//
// Construction is explicit so a raw QString can't sneak in by accident; the
// underlying value is exposed via `toString()` for JSON/settings boundaries.

namespace detail {

template <typename Tag>
class StringId {
public:
    StringId() = default;
    explicit StringId(QString value) : m_value(std::move(value)) {}

    bool isValid() const { return !m_value.isEmpty(); }
    const QString& toString() const { return m_value; }

    bool operator==(const StringId& other) const { return m_value == other.m_value; }
    bool operator!=(const StringId& other) const { return m_value != other.m_value; }

private:
    QString m_value;
};

template <typename Tag>
inline size_t qHash(const StringId<Tag>& id, size_t seed = 0) noexcept {
    return qHash(id.toString(), seed);
}

} // namespace detail

struct TemplateIdTag {};
struct ThemeIdTag {};
using TemplateId = detail::StringId<TemplateIdTag>;
using ThemeId = detail::StringId<ThemeIdTag>;
