# Navigating presets via Midi

Preset supports navigation using a Midi keyboard or controller.

## Keyboard navigation

Keyboard navigation uses Midi Note On to trigger a navigation action.
Press the key to take the action.
The concept is the same as using keyswitches in DAWs/VSTs for selecting parameters.

| Action | Description |
| -- | -- |
| Select | Send the preset at the cursor to the instrument. |
| Page&nbsp;mode | Prev/Next move the cursor by pages. |
| Index&nbsp;mode | Prev/Next move the cursor by preset within the page. |
| Toggle&nbsp;mode | Toggles between page and index mode. |
| Previous | Move cursor to the previous page or index. |
| Next | Move cursor to the next page or index. |
| Range | Move the cursor to the item given by the distance from the *First* key. |

You can choose one key for toggling the page/index mode, or two keys, each setting a particular mode.

Configure *Toggle* mode by selecting the same key for both *Page* and *Index*, or by configuring only one of them, leaving the other blank.

Clicking between square brackets activates MIDI learn for that item.
When learn mode is active, press a key to assign that note.
To clear an entry, press **ESC**.
To exit learn mode click empty space on the dialog.

Range is optional and can be left unconfigured (empty).
In page mode, a range action goes directly to a page, and in Index mode goes to the zero-based offset from the start of the page.
Range is configured by specifying the *First* key.
All the remaining keys on the keyboard to the right of the selected *First* key are active.

One typically uses the lowest keys on the keyboard for keyswitches.

### Quick Config

For convenience, there are two quick configurations.
From an empty configuration, click in the Select learn box, and press:

| Note | Configuration |
| -- | -- |
| **F** | **F** select, **F#** page mode **Ab** index mode, **G** prev, **A** next, **Bb** first |
| **C** | **C** select, **D** toggle mode, **C#** Prev, **Eb** Next, **E** first |

If you don't like the quick configuration, simply click in the learn boxes and play the keys you want for each function.

## Controller navigation

Knobs and faders are interchangeable from the MIDI perspective, so we'll always refer to "knobs" here.

4-button cursor row: Select (m), Toggle mode (t), Prev (m), Next (m).

5-button cursor diamond: Select (m), Prev page (m), Next page (m), Prev index (m), Next index (m).

1 button, 2 knobs/faders: Select (m), Page (k), Index (k).

3 knobs: Select, Page, Index