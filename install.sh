#!/bin/sh
set -eu

SERVICE_NAME="chain-bot"
INSTALL_DIR="/opt/chain-bot"
CONFIG_DIR="${INSTALL_DIR}/config"
LOG_DIR="${INSTALL_DIR}/logs"
MODULE_DIR="${INSTALL_DIR}/modules"
BINARY_SOURCE="build/chain-bot"
IRC_MODULE_SOURCE="build/modules/chain_bot_irc.so"
SLACK_MODULE_SOURCE="build/modules/chain_bot_slack.so"
CONFIG_SOURCE="config/chain_bot.json"
SERVICE_SOURCE="systemd/chain-bot.service"
SERVICE_TARGET="/etc/systemd/system/chain-bot.service"

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: install.sh must be run as root." >&2
    exit 1
fi

for required_file in "${BINARY_SOURCE}" "${IRC_MODULE_SOURCE}" "${SLACK_MODULE_SOURCE}" "${CONFIG_SOURCE}" "${SERVICE_SOURCE}"; do
    if [ ! -f "${required_file}" ]; then
        echo "ERROR: ${required_file} not found. Run make first." >&2
        exit 1
    fi
done

if ! getent group chain-bot >/dev/null 2>&1; then
    groupadd --system chain-bot
fi

if ! id chain-bot >/dev/null 2>&1; then
    useradd --system \
        --gid chain-bot \
        --home-dir "${INSTALL_DIR}" \
        --shell /usr/sbin/nologin \
        chain-bot
fi

install -d -o root -g chain-bot -m 0750 "${INSTALL_DIR}"
install -d -o root -g chain-bot -m 0750 "${CONFIG_DIR}"
install -d -o chain-bot -g chain-bot -m 0750 "${LOG_DIR}"
install -d -o root -g chain-bot -m 0750 "${MODULE_DIR}"
install -o root -g root -m 0755 "${BINARY_SOURCE}" "${INSTALL_DIR}/chain-bot"
install -o root -g chain-bot -m 0750 "${IRC_MODULE_SOURCE}" "${MODULE_DIR}/chain_bot_irc.so"
install -o root -g chain-bot -m 0750 "${SLACK_MODULE_SOURCE}" "${MODULE_DIR}/chain_bot_slack.so"

if [ ! -f "${CONFIG_DIR}/chain_bot.json" ]; then
    install -o root -g chain-bot -m 0640 "${CONFIG_SOURCE}" "${CONFIG_DIR}/chain_bot.json"
    echo "Installed initial configuration to ${CONFIG_DIR}/chain_bot.json"
else
    echo "Preserving existing ${CONFIG_DIR}/chain_bot.json"
fi

install -o root -g root -m 0644 "${SERVICE_SOURCE}" "${SERVICE_TARGET}"

systemctl daemon-reload
systemctl enable "${SERVICE_NAME}.service"

echo "Chain Bot installed."
echo "Service: ${SERVICE_NAME}.service"
echo "Config:  ${CONFIG_DIR}/chain_bot.json"
echo "Modules: ${MODULE_DIR}"
echo "Log:     ${LOG_DIR}/chain-bot.log"
echo "Start:   systemctl start ${SERVICE_NAME}"
echo "Stop:    systemctl stop ${SERVICE_NAME}"
echo "Restart: systemctl restart ${SERVICE_NAME}"
echo "Status:  systemctl status ${SERVICE_NAME}"
echo "Journal: journalctl -u ${SERVICE_NAME} -f"
