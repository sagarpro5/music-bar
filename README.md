# YouTube Music Bar — AI Branch

This branch contains the AI-assisted development of the YouTube Music Bar project.

## Project goal

The project detects music playing on YouTube and displays a lightweight GTK music bar on the desktop.

## Architecture

The project has three main components:

1. **Violentmonkey userscript** — detects the active YouTube video, classifies whether it is music, and sends the current state to the local server.
2. **`server.py`** — receives and validates the state and writes `state.json`.
3. **`music_bar.c`** — reads `state.json` and shows or hides the GTK bar.

## Shared state

The state contains:

- `active` — whether a relevant YouTube video is active.
- `playing` — whether the video is currently playing.
- `music` — whether the detector classifies the video as music.
- `title` — the current video title.
- `channel` — the YouTube channel.
- `url` — the current video URL.

When there is no relevant active video, the state should be reset to an inactive state with an empty title.

## Music detection

The detector should use multiple signals instead of relying on one keyword. Useful signals include video title, channel name, artist information, lyrics indicators, official music-video indicators, remixes, covers, soundtracks, and YouTube metadata.

The classifier must be tested against both music and normal videos to reduce false positives and false negatives.

## Multiple YouTube tabs

The system should correctly handle multiple YouTube tabs. A normal video in one tab must not incorrectly override a relevant music video in another tab.

The userscript should also handle:

- switching between YouTube tabs
- pausing and resuming videos
- navigating between videos
- closing a video or tab
- YouTube single-page navigation
- inactive or unavailable videos

## Music bar behavior

The GTK bar should be visible only when:

- a YouTube video is active
- the video is playing
- the detector classifies it as music
- a valid title exists

Otherwise the bar should be hidden and stale title information should not remain visible.

## Development principles

- Preserve working functionality.
- Prefer small, testable changes over unnecessary rewrites.
- Treat browser console output and server logs as evidence.
- Do not claim a fix is verified until it has been tested.
- Keep the userscript, server, and GTK client compatible with the same state format.
- Test real music videos as well as normal YouTube videos.

## Testing

Important test cases include:

- official music videos
- Hindi and Bollywood songs
- independent music
- lyric videos
- remixes
- covers
- soundtracks
- music channels
- gaming videos
- tutorials
- news videos
- podcasts
- normal videos containing music
- multiple YouTube tabs
- switching tabs
- pausing videos
- closing videos

## Branch

`ai`

## Status

Active development and testing branch for improving music detection and the reliability of the YouTube Music Bar.
