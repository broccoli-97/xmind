# YMind Markdown Import Format

YMind's **File → Import from Markdown...** accepts a small, strict subset of
Markdown so that the structure of the resulting mind map is always
unambiguous. If the file contains anything outside this subset, the import is
rejected with line-numbered error messages and the current map is left
untouched.

This document is the single source of truth for the format. It is suitable
both as a human reference and as a prompt fragment for AI tools that generate
input for YMind.

## TL;DR

```markdown
# Optional root title

- First child of root
  - A grandchild (2 spaces deeper)
    - A great-grandchild (4 spaces deeper)
- Second child of root
- Third child of root
  - Another grandchild
```

That's it. One optional `# Heading` line at the top, followed by an unordered
list. Two spaces per nesting level. No other Markdown features.

## Rules

1. **Root node**
   - The file may begin with exactly one `# Title` line. That title becomes
     the root node of the mind map.
   - If no `# Title` is present, the first unindented bullet becomes the root
     and its siblings become the root's children.
   - Headings other than `#` (`##`, `###`, ...) are **not allowed**.

2. **Bullets**
   - Each non-root node is an unordered list item starting with `- `, `* `,
     or `+ ` followed by a single space and the node text.
   - Ordered lists (`1.`, `2)`, ...) are **not allowed**.

3. **Indentation**
   - Use **2 spaces per nesting level**. No tabs.
   - The indent count must be a multiple of 2.
   - You cannot skip levels (e.g. jumping from 0 to 4 spaces).

4. **Line content**
   - One node per line. No multi-line items.
   - Inline `**bold**`, `*italic*`, `` `code` ``, and `[text](url)` links are
     accepted; only the visible text is kept.
   - Blank lines are allowed everywhere and ignored.
   - `<!-- HTML comments -->` are silently ignored (useful for metadata that
     should not appear in the map).

5. **Anything else is rejected**, including:
   - Headings `##`–`######`
   - Setext headings (`Title` followed by `===` or `---`)
   - Horizontal rules (`---`, `***`, `___`)
   - Fenced code blocks (` ``` ` or `~~~`)
   - Blockquotes (`> ...`)
   - Tables (any line containing `|`)
   - Raw HTML tags
   - Stray paragraph text after the structure has begun

## Examples

### Valid

```markdown
# Trip to Kyoto

- Day 1
  - Fly in
  - Check in to hotel
- Day 2
  - Fushimi Inari
    - Bring water
  - Gion at night
- Day 3
  - Arashiyama bamboo grove
```

```markdown
- Project
  - Backend
    - API
    - Database
  - Frontend
    - Components
    - Routing
```

### Rejected

```markdown
# Trip
## Day 1               <-- ## headings not supported
- Pack
```

```markdown
# Trip
- Day 1
	- Pack             <-- tab indentation
```

```markdown
# Trip
- Day 1
- Day 2
   - Hike             <-- 3-space indent (not a multiple of 2)
```

```markdown
# Trip
1. Day 1              <-- ordered list
```

```markdown
# Trip
- Day 1
- Day 2
| col | col |         <-- table
```

## Notes for AI tools

When generating input for YMind, follow these constraints exactly:

- Emit one `# Title` line, then one blank line.
- Use `- ` (hyphen + space) for every bullet.
- Indent nested bullets by exactly two spaces per level relative to their
  parent.
- Do not insert headings, code fences, tables, blockquotes, HTML, or any
  other Markdown features.
- Keep each bullet's text on a single line. If the source content has
  paragraphs, summarize them into a single line per node.
- Prefer concise node labels (a few words to a short sentence).
