#!/usr/bin/env bash
# Generate a categorized changelog from Conventional Commits between two refs.
#
# Usage: generate-changelog.sh <prev_ref> <current_ref>
#
# - <prev_ref>    : previous tag (e.g. v1.2.0). If empty, lists every commit
#                    reachable from <current_ref>.
# - <current_ref> : tag/ref the changelog is generated FOR (e.g. v1.3.0).
#
# Env vars used:
#   GITHUB_REPOSITORY  "owner/repo" — used to build commit + compare URLs.
#
# Recognized commit prefixes (from CLAUDE.md plus a few common ones):
#   feat, fix, bugfix, refactor, perf, style, docs, test, build, ci, chore
# Commits whose subject does not match the pattern fall under "Other Changes".
# Merge commits ("Merge pull request..." / "Merge branch...") are skipped.

set -euo pipefail

PREV="${1:-}"
CURRENT="${2:-HEAD}"

if [[ -z "$PREV" ]]; then
    RANGE="$CURRENT"
else
    RANGE="${PREV}..${CURRENT}"
fi

REPO="${GITHUB_REPOSITORY:-}"

# Category metadata: prefix -> display heading. Order below controls section order.
declare -A HEADINGS=(
    [feat]="### Features"
    [fix]="### Bug Fixes"
    [perf]="### Performance"
    [refactor]="### Refactoring"
    [style]="### Style"
    [docs]="### Documentation"
    [test]="### Tests"
    [build]="### Build"
    [ci]="### CI"
    [chore]="### Chores"
)
CATEGORY_ORDER=(feat fix perf refactor style docs test build ci chore)

declare -A BUCKETS
OTHER=""

format_link() {
    local hash="$1"
    local short="${hash:0:7}"
    if [[ -n "$REPO" ]]; then
        printf '([`%s`](https://github.com/%s/commit/%s))' "$short" "$REPO" "$hash"
    else
        printf '(`%s`)' "$short"
    fi
}

MERGE_RE='^Merge (pull request|branch|remote-tracking)'
COMMIT_RE='^(feat|fix|bugfix|perf|refactor|style|docs|test|build|ci|chore)(\(([^)]+)\))?(!)?:[[:space:]]*(.+)$'

while IFS=$'\t' read -r HASH SUBJECT; do
    # Skip merge commits — they re-list their constituent commits.
    if [[ "$SUBJECT" =~ $MERGE_RE ]]; then
        continue
    fi

    LINK="$(format_link "$HASH")"

    # bugfix: <desc> → treat as fix
    # feat(scope)!: <desc> → breaking change marker
    if [[ "$SUBJECT" =~ $COMMIT_RE ]]; then
        TYPE="${BASH_REMATCH[1]}"
        SCOPE="${BASH_REMATCH[3]:-}"
        BREAKING="${BASH_REMATCH[4]:-}"
        DESC="${BASH_REMATCH[5]}"
        # Normalize bugfix → fix
        [[ "$TYPE" == "bugfix" ]] && TYPE="fix"

        if [[ -n "$SCOPE" ]]; then
            LINE="- **${SCOPE}:** ${DESC} ${LINK}"
        else
            LINE="- ${DESC} ${LINK}"
        fi
        if [[ -n "$BREAKING" ]]; then
            LINE="${LINE} ⚠️ **BREAKING**"
        fi
        BUCKETS[$TYPE]="${BUCKETS[$TYPE]:-}${LINE}"$'\n'
    else
        OTHER+="- ${SUBJECT} ${LINK}"$'\n'
    fi
done < <(git log --no-merges --pretty=format:'%H%x09%s' "$RANGE")

# --- Output -----------------------------------------------------------------

if [[ -n "$PREV" ]]; then
    echo "## What's Changed since ${PREV}"
else
    echo "## What's Changed"
fi
echo

for TYPE in "${CATEGORY_ORDER[@]}"; do
    if [[ -n "${BUCKETS[$TYPE]:-}" ]]; then
        echo "${HEADINGS[$TYPE]}"
        echo
        printf '%s' "${BUCKETS[$TYPE]}"
        echo
    fi
done

if [[ -n "$OTHER" ]]; then
    echo "### Other Changes"
    echo
    printf '%s' "$OTHER"
    echo
fi

if [[ -n "$PREV" && -n "$REPO" ]]; then
    echo "**Full Changelog**: https://github.com/${REPO}/compare/${PREV}...${CURRENT}"
fi
