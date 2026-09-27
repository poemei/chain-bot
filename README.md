# Chain Bot

The STNC Chain Project IRC Bot.

Chain Bot is a Linux-only ISO C17 companion service for STN Chain. It is intended to run on the same server as `stn-chain`, observe the local Chain through STNC, and publish operational events to IRC.

## Initial Scope

- Announce new accepted blocks
- Alert on Chain errors and recovery
- Announce block wins when authoritative information is available
- Report reorganizations and other relevant Chain events
- Support bounded read-only IRC commands

## IRC

- Network: Libera.Chat
- Server: `irc.libera.chat:6697`
- TLS: required
- Channel: `#STNC-Chain`

## Chain Connection

Chain Bot connects to the local STN Chain node through STNC v2.

- Host: `127.0.0.1`
- Port: `18473`

Chain Bot is a Chain consumer. It does not implement or determine STN Chain consensus.

## Architecture

The bot uses the STN-LABZ ABI v1.4 from the sibling `../ABI` directory and supports modular capabilities following the established STN-LABZ application module model.

The host remains small: lifecycle, configuration, STNC connectivity, IRC/TLS connectivity, and module dispatch. Additional capabilities belong in modules.

**Small. Deterministic. Easy to use.**
