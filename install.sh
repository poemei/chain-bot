#!/bin/sh
set -eu

SERVICE_NAME="chain-bot"
INSTALL_DIR="/opt/chain-bot"
CONFIG_DIR="${INSTALL_DIR}/config"
BINARY_SOURCE="build/chain-bot"
CONFIG_SOURCE="config/chain_bot.json"
SERVICE_SOURCE="systemd/chain-bot.service"
SERVICE_TARGET="/etc/systemd/system/chain-bot.service"

if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: install.sh must be run as root." >&2
    exit 1
fi

if [ ! -x "${BINARY_SOURCE}" ]; then
    echo "ERROR: ${BINARY_SOURCE} not found or not executable. Run make first." >&2
    exit 1
fi

if [ ! -f "${CONFIG_SOURCE}" ]; then
    echo "ERROR: ${CONFIG_SOURCE} not found." >&2
    exit 1
fi

if [ ! -f "${SERVICE_SOURCE}" ]; then
    echo "ERROR: ${SERVICE_SOURCE} not found." >&2
    exit 1
fi

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
install -o root -g root -m 0755 "${BINARY_SOURCE}" "${INSTALL_DIR}/chain-bot"

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
echo "Start:   systemctl start ${SERVICE_NAME}"
echo "Stop:    systemctl stop ${SERVICE_NAME}"
echo "Restart: systemctl restart ${SERVICE_NAME}"
echo "Status:  systemctl status ${SERVICE_NAME}"
echo "Logs:    journalctl -u ${SERVICE_NAME} -f"
