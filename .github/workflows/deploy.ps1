$build = "test"
$repo = ($env:GITHUB_REPOSITORY -Split '/')[1] ?? $env:GITHUB_REPOSITORY
&ssh "${env:SSH_USER}@${env:SSH_HOST}" (@(
    "rm -rf ~/staging/${repo}",
    "mkdir -p ~/staging/${repo}"
) -Join '&&')
&scp -r './staging/*' "${env:SSH_USER}@${env:SSH_HOST}:~/staging/${repo}"
&ssh "${env:SSH_USER}@${env:SSH_HOST}" (@(
    "mv -f ~/staging/${repo}/files/* ${env:FILES_PATH}/",
    "mv -f ${env:SYMBOLS_PATH}/ ${env:SYMBOLS_PATH}~/",
    "mv -f ~/staging/${repo}/symbols/ ${env:SYMBOLS_PATH}/",
    "rm -r ${env:SYMBOLS_PATH}~/"
    "rm -r ~/staging/${repo}"
) -Join '&&')
