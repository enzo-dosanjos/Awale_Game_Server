# Awale_Game_Server

## 1. Overview

This project implements a networked Awale game with:

- A TCP server managing:
    - Accounts (sign up / login)
    - Challenges and games between players
    - Game saving / loading
    - Spectators
    - Profiles, friends, and stats
- A TCP client (terminal) to interact with the server.

The server is multi\-client and text\-command based. The same commands are documented in the `HELP` command on the server.

---

## 2. Requirements

- OS: Linux (tested), Windows
- Compiler: `gcc` with C18 support
- Tools: `make`

---

## 3. Build Instructions

From the project root (where the `makefile` is located):

```bash
# Clean and build server and client
make

# Or build only the server
make server

# Or build only the client
make client

# Debug build (adds -g and MAP)
make debug

# Clean binaries and build directory
make clean
```

This produces two executables in the project root:

- `Awale` → the server
- `Client` → the client

---

## 4. Running the Server

Start the server on the default port (`1977`):

```bash
./Awale
```

The server listens on `PORT 1977` (defined in `constants.h`).

Make sure the port is not blocked by a firewall.

---

## 5. Running the Client

On the same machine:

```bash
./Client 127.0.0.1
```

From another machine on the network:

```bash
./Client <server_ip_address>
```

Replace `<server_ip_address>` with the actual IP of the machine running `Awale`.

---

## 6. General Usage (Client Side)

When the client starts, you are in the *lobby* (not logged in).

### 6\.1 Lobby Commands (not logged in)

- `HELP`  
  Show all available commands (for lobby or logged\-in mode).

- `MSG <message>`  
  Send a message to the lobby chat.

- `LOGIN <username> <password>`  
  Log into an existing account.

- `SIGNUP <username> <password>`  
  Create a new account and automatically connect.

After `LOGIN` or `SIGNUP`, the socket leaves the lobby and becomes a connected client.

---

## 7. Logged\-in Commands

When logged in, you can use all the following commands.

### 7\.1 General Commands

- `HELP`  
  Show the global help (all commands).

- `LIST`  
  List currently connected users and show if they are *in game*.

- `LISTGAMES`  
  List all ongoing games with their game IDs and players.

- `QUIT`  
  Disconnect from the server.  
  If you are in a game, the game is saved and can be resumed when all players are connected.

---

### 7\.2 Public & Private Messaging

- `MSG <message>`  
  Send a message to the general chat (all connected players).

- `MSG @<username> <message>`  
  Send a private message to a specific connected user.

---

### 7\.3 Challenges & Game Creation

- `CHALLENGE <username>`  
  Send a game challenge to a connected user.
    - You cannot challenge yourself.
    - There is a limit on pending challenges sent and received.

- `ACCEPT <username>`  
  Accept a pending challenge from `username`.  
  Flow:
    1. Pending challenge is checked and removed.
    2. You are asked to enter the game rotation:  
       `0` for counter\-clockwise, `1` for clockwise.
    3. A new game is created and all players are notified.
    4. The board grid is displayed and whose turn it is.

- `DECLINE <username>`  
  Decline a pending challenge from `username`.

- `SEEPENDINGREQ`  
  List usernames who have challenged you.

- `SEESENTREQ`  
  List usernames you have challenged.

- `CLEARPENDINGREQ`  
  Clear all received pending challenges.

- `CLEARSENTREQ`  
  Clear all sent pending challenges.

- `REMOVESENTREQ <username>`  
  Cancel the challenge you sent to `username`.

---

### 7\.4 Playing a Game

When a game is started or loaded, both players see the game grid.

- `MOVE <house>`  
  Play a move from the chosen house:
    - Must be your turn.
    - Illegal moves are rejected with an error message.
    - On success:
        - The move is applied.
        - The board is reprinted for both players and all viewers.
        - Move history is recorded.

- `ENDGAME`  
  Propose to end the current game by mutual agreement.
    - Sets a flag for the proposer.
    - Sends a prompt to the opponent:  
      `"The opponent suggests ending this game. ACCEPTEND?"`

- `ACCEPTEND`  
  Accept your opponent's endgame proposal.  
  If accepted by all players, the game is ended and the result is computed.

- `LASTGAME`  
  Load your last unfinished game:
    - Only works if there is a stored game session for you.
    - All players must be connected and not already in another game.
    - The game resumes and the board is displayed.

- `SAVEGAME`  
  Save the current game to a file and send it to you:
    - On the server, a directory like `SAVE_DIR` is created.
    - A text file `game_<id>.txt` is generated (with moves, chat, etc.).
    - The server sends the file between markers:
        - `BEGIN_SAVED_GAME <filename>`
        - file content
        - `END_SAVED_GAME`
    - The client receives it and saves it locally under the given file name.

**End of Game Flow**

When the game ends (by rules or `ENDGAME/ACCEPTEND`), the server:

- Calculates the winner.
- Asks each player if they want to save the game (`yes`/`y` to save).
- Updates both players' stats (see Section 9).
- Clears `gameId` for players.
- Frees and removes the `GameSession`.

---

### 7\.5 Spectating Games

- `LISTGAMES`  
  Shows ongoing games and their IDs.

- `WATCH <gameId>`  
  Watch a running game as a viewer:
    - You cannot watch if you are currently playing.
    - You cannot exceed `MAX_VIEWERS` for that game.
    - If the game or players are private, you must be in their friends list.

- `MSGGAME <message>`  
  Send a message to the *current* game chat:
    - Works if you are a player or a viewer of that game.
    - The message is broadcast to:
        - all players
        - all viewers of that game
    - Chat history is recorded in the game session.

---

### 7\.6 Profile & Social Features

- `BIO <text>`  
  Update your bio. Stored in the `Client` structure.

- `SHOWBIO [username]`  
  Show the bio:
    - If no `username` is given, shows *your* bio.
    - If `username` is given:
        - If that profile is private, you must be a friend to see it.

- `SHOWSTATS [username]`  
  Show stats (see Section 9):
    - If no `username`, shows your stats.
    - If `username` is given:
        - If profile is private, you must be a friend.

- `ADDFRIEND <username>`  
  Add `username` to your friends list:
    - Cannot add yourself.
    - User must exist.
    - You cannot add the same friend twice.
    - Limited by `MAX_FRIENDS`.

- `REMOVEFRIEND <username>`  
  Remove `username` from your friends list:
    - If not a friend, an error is returned.

- `SHOWFRIENDS [username]`  
  Show the list of friends:
    - If no `username`, shows your friends.
    - If `username` is given:
        - If their friends list is private, you must be their friend.

- `SETPRIVACY <true|false>`  
  Set your privacy setting:
    - `true` → your bio, stats, games, etc. are restricted to friends.
    - `false` → publicly visible (within the server).

---

## 8. Game History and Saving

Each `GameSession` keeps:

- Move history:
    - Move number
    - Player index
    - House played
    - Timestamp
    - Board state (grid) after the move

- Game chat history:
    - Sender
    - Text
    - Timestamp

When saving a game (`SAVEGAME` or end\-of\-game save), the server writes:

- A header with:
    - Game ID
    - Players and scores
    - Current player
    - Number of moves
- Move history (\+ boards)
- Game chat history

The client receives the file and writes it to disk when `BEGIN_SAVED_GAME` / `END_SAVED_GAME` are received.

---

## 9. Player Statistics

For each account, the server tracks:

- `gamesPlayed` → total games played
- `gamesWon` → number of wins
- `gamesLost` → number of losses
- `gamesDrawn` → number of draws
- `averageMovesToWin` → average number of moves (per player) in games you won
- `totalSeedsCollected` → sum of seeds collected across all games

These stats are updated at the end of every game and shown by the `SHOWSTATS` command.
