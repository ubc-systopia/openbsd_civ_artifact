#!/bin/bash
# install-openclaw.sh -- unattended OpenClaw install on Ubuntu 24.04, wired to
# Claude Sonnet 4.6 through an Anthropic API key.
#
#   1. put your key in openclaw.env (same directory as this script)
#   2. ./install-openclaw.sh
#
# What it does (each step is idempotent, re-running is safe):
#   - runs the official installer (curl https://openclaw.ai/install.sh), which
#     installs Node.js 24 from NodeSource via apt (sudo) and OpenClaw via npm
#     into ~/.npm-global (PATH is added to ~/.bashrc)
#   - runs the onboarding wizard non-interactively with the Anthropic key,
#     which stores the key in OpenClaw's auth store, writes
#     ~/.openclaw/openclaw.json, and installs a systemd *user* service
#     (openclaw-gateway) with lingering so it survives logout/reboot
#   - sets the default model, disables OpenAI-only memory-search embeddings,
#     restarts the gateway and sends one real test message to the model
#
# Needs: Ubuntu 24.04, a normal (non-root) user with sudo, internet access.
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ENV_FILE=${OPENCLAW_ENV_FILE:-$SCRIPT_DIR/openclaw.env}

info() { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
die()  { printf '\n\033[1;31mERROR: %s\033[0m\n' "$*" >&2; exit 1; }

# --- 0. preflight ------------------------------------------------------------
[ "$(id -u)" -ne 0 ] || die "run this as a regular user with sudo, not as root"
command -v sudo >/dev/null || die "sudo is not installed"
sudo -n true 2>/dev/null || sudo -v || die "this user needs sudo"
. /etc/os-release 2>/dev/null || true
[ "${ID:-}" = ubuntu ] || die "this script targets Ubuntu (found ${PRETTY_NAME:-unknown})"
case "${VERSION_ID:-}" in 24.*) ;; *) echo "WARNING: tested on Ubuntu 24.04, found $VERSION_ID; continuing" >&2 ;; esac

# --- 1. read settings from openclaw.env --------------------------------------
[ -f "$ENV_FILE" ] || die "settings file not found: $ENV_FILE"
getvar() {  # getvar NAME -> value of NAME="..." / NAME='...' / NAME=... (export ok)
	sed -n -E "s/^[[:space:]]*(export[[:space:]]+)?$1[[:space:]]*=[[:space:]]*//p" "$ENV_FILE" \
		| tail -n 1 | sed -E "s/^\"(.*)\"[[:space:]]*\$/\1/; s/^'(.*)'[[:space:]]*\$/\1/; s/[[:space:]]+\$//"
}
API_KEY=$(getvar ANTHROPIC_API_KEY)
# also accept a bare key pasted on a line of its own
[ -n "$API_KEY" ] || API_KEY=$(grep -m1 -oE '^[[:space:]]*sk-ant-[A-Za-z0-9_-]+' "$ENV_FILE" | tr -d '[:space:]' || true)
[ -n "$API_KEY" ] || die "ANTHROPIC_API_KEY is empty in $ENV_FILE -- paste your key between the quotes"
case "$API_KEY" in sk-ant-*) ;; *) die "ANTHROPIC_API_KEY in $ENV_FILE does not look like an Anthropic key (expected sk-ant-...)" ;; esac
MODEL=$(getvar OPENCLAW_MODEL);          MODEL=${MODEL:-anthropic/claude-sonnet-4-6}
BIND=$(getvar OPENCLAW_GATEWAY_BIND);    BIND=${BIND:-loopback}
case "$BIND" in loopback|lan) ;; *) die "OPENCLAW_GATEWAY_BIND must be loopback or lan (got '$BIND')" ;; esac
WORKSPACE=$(getvar OPENCLAW_WORKSPACE);  WORKSPACE=${WORKSPACE:-$HOME/openclaw-workspace}
WORKSPACE=${WORKSPACE/#\~/$HOME}; WORKSPACE=${WORKSPACE//\$HOME/$HOME}
WS_ONLY=$(getvar OPENCLAW_WORKSPACE_ONLY); WS_ONLY=${WS_ONLY:-false}
case "$WS_ONLY" in true|false) ;; *) die "OPENCLAW_WORKSPACE_ONLY must be true or false (got '$WS_ONLY')" ;; esac

# --- 2. official installer (Node.js + OpenClaw), no wizard --------------------
export PATH="$HOME/.npm-global/bin:$PATH"
if command -v openclaw >/dev/null 2>&1; then
	# Re-running the official installer over an existing install takes its
	# upgrade path (stops the gateway, runs a repair doctor) -- not wanted here.
	info "OpenClaw already installed ($(openclaw --version)); skipping the installer. To upgrade: npm i -g openclaw@latest"
else
	info "Installing OpenClaw (official installer; this takes a couple of minutes)"
	export DEBIAN_FRONTEND=noninteractive NEEDRESTART_MODE=a
	curl -fsSL --proto '=https' --tlsv1.2 https://openclaw.ai/install.sh \
		| bash -s -- --no-prompt --no-onboard
	hash -r
	command -v openclaw >/dev/null || die "openclaw is not on PATH after install"
	info "Installed: $(openclaw --version)"
fi

# --- 3. onboarding: Anthropic API key + gateway as a systemd user service ------
info "Configuring OpenClaw (Anthropic API key, workspace $WORKSPACE, gateway service)"
mkdir -p "$WORKSPACE"
openclaw onboard --non-interactive --accept-risk \
	--auth-choice apiKey --anthropic-api-key "$API_KEY" \
	--workspace "$WORKSPACE" \
	--gateway-bind "$BIND" --install-daemon --skip-health

# --- 4. model + settings ----------------------------------------------------
info "Setting default model to $MODEL"
openclaw models set "$MODEL"
# Semantic memory search defaults to OpenAI embeddings; with only an Anthropic
# key it just logs warnings, so turn it off.  Re-enable later with an OpenAI
# key:  openclaw config set memory.search.enabled true
openclaw config set memory.search.enabled false >/dev/null
openclaw config set memory.search.rememberAcrossConversations false >/dev/null
# Workspace: --workspace above only applies to a brand-new config, so set it
# explicitly too (re-runs).  The agent seeds AGENTS.md/SOUL.md/... into it on
# its first run.
openclaw config set agents.defaults.workspace "$WORKSPACE" >/dev/null
openclaw config set agents.entries.main.workspace "$WORKSPACE" >/dev/null
# File tools (read/write/edit) scope.  Unset = OpenClaw default = anywhere this
# user can reach; "true" confines them to the workspace.  (Authoring an explicit
# "false" makes the gateway log a security warning, hence unset.)  Sandboxing
# stays off, so exec runs on this host as this user.
if [ "$WS_ONLY" = true ]; then
	openclaw config set tools.fs.workspaceOnly true >/dev/null
else
	openclaw config unset tools.fs.workspaceOnly >/dev/null 2>&1 || true
fi
openclaw gateway restart >/dev/null

# --- 5. verify ---------------------------------------------------------------
info "Verifying: gateway service"
systemctl --user is-enabled --quiet openclaw-gateway || die "openclaw-gateway user service was not installed"
for i in $(seq 1 60); do openclaw health --json >/dev/null 2>&1 && break; sleep 1; done
openclaw health --json >/dev/null 2>&1 || die "gateway is not answering (see: journalctl --user -u openclaw-gateway)"
openclaw models status | sed -n '/^Default/p'

info "Verifying: sending a test message to $MODEL"
out=$(openclaw agent --timeout 120 --json -m 'Reply with exactly one word: OK' 2>&1) || {
	printf '%s\n' "$out" | tail -5
	die "the test message failed. If the error says HTTP 401 your API key is wrong: fix it in $ENV_FILE and re-run this script."
}
reply=$(printf '%s' "$out" | python3 -c '
import json,sys
def walk(o):
    if isinstance(o,dict):
        for k,v in o.items():
            if k=="text" and isinstance(v,str): return v
            r=walk(v)
            if r: return r
    elif isinstance(o,list):
        for i in o:
            r=walk(i)
            if r: return r
try: print(walk(json.loads(sys.stdin.read())) or "")
except Exception: print("")' 2>/dev/null || true)
echo "model replied: ${reply:-<no text found; raw output: $out>}"

# --- done --------------------------------------------------------------------
UI_URL=$(openclaw dashboard --no-open --json 2>/dev/null | python3 -c 'import json,sys; print(json.load(sys.stdin).get("url",""))' 2>/dev/null || true)
IP=$(hostname -I 2>/dev/null | awk '{print $1}')
printf '\n\033[1;32mOpenClaw is installed and running with %s.\033[0m\n' "$MODEL"
cat <<EOD

  Workspace folder:       $WORKSPACE  (file tools confined to it: $WS_ONLY)
  Chat in the terminal:   openclaw tui
  One-off message:        openclaw agent -m "hello"
  Health check:           openclaw doctor
  Gateway service:        systemctl --user status openclaw-gateway
  Control UI (browser):   ${UI_URL:-run: openclaw dashboard --no-open}
EOD
if [ "$BIND" = loopback ]; then
	echo "      from another machine:  ssh -L 18789:127.0.0.1:18789 $USER@${IP:-<this-host>}   then open the URL above"
else
	echo "      (gateway bound to LAN: replace 127.0.0.1 with ${IP:-<this-host>} in the URL)"
fi
cat <<EOD

Open a new shell (or run: source ~/.bashrc) so 'openclaw' is on your PATH.
Config: ~/.openclaw/openclaw.json    Logs: /tmp/openclaw/
EOD
