#!/bin/sh
set -ev
if [ "${TRAVIS_PULL_REQUEST}" != "false" ]; then
    curl -H "Authorization: token ${COVERAGE_POSTER_API_TOKEN}" \
    -X POST -d "{\"body\": \"\`\`\`\n$(cat coverage/coverage.txt | awk -v ORS='\\n' '1')\`\`\`\"}" \
    "https://api.github.com/repos/${TRAVIS_REPO_SLUG}/issues/${TRAVIS_PULL_REQUEST}/comments";
fi