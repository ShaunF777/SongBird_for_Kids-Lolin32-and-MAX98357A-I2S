#include "WebModule.h"
#include "AudioModule.h"
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

namespace WebModule {

static AsyncWebServer server(80);
static Settings* g_settings = nullptr;
static File g_uploadFile;

// The settings/upload page. %PLACEHOLDER% markers are filled in
// by handleRoot() with the current saved settings.
static const char PAGE_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>SongBird</title>
  <style>
    :root {
      --card-bg: rgba(255, 255, 255, 0.92);
      --accent: #ff6b6b;
      --accent-dark: #e64848;
      --text: #33334d;
    }
    * { box-sizing: border-box; }
    body {
      font-family: -apple-system, "Segoe UI", Roboto, sans-serif;
      max-width: 420px;
      margin: 0 auto;
      padding: 1.5em 1em 3em;
      color: var(--text);
      background: linear-gradient(160deg, #a1c4fd 0%, #c2e9fb 45%, #fbc2eb 100%);
      min-height: 100vh;
    }
    h1 {
      text-align: center;
      font-size: 2.4em;
      margin: 0.2em 0 0;
      font-weight: 900;
      background: linear-gradient(90deg, #ff6b6b, #feca57, #48dbfb, #ff6b6b);
      background-size: 300% auto;
      -webkit-background-clip: text;
      background-clip: text;
      color: transparent;
      animation: rainbow 4s linear infinite;
    }
    @keyframes rainbow {
      to { background-position: 300% center; }
    }
    .bird {
      display: block;
      text-align: center;
      font-size: 3.5em;
      animation: bob 2s ease-in-out infinite;
    }
    @keyframes bob {
      0%, 100% { transform: translateY(0) rotate(-8deg); }
      50%      { transform: translateY(-14px) rotate(8deg); }
    }
    .subtitle {
      text-align: center;
      opacity: 0.75;
      margin: 0.2em 0 1.5em;
    }
    .card {
      background: var(--card-bg);
      border-radius: 18px;
      padding: 1.2em 1.4em 1.4em;
      margin-bottom: 1.2em;
      box-shadow: 0 8px 24px rgba(60, 60, 100, 0.15);
      backdrop-filter: blur(4px);
    }
    .card h2 {
      margin: 0 0 0.8em;
      font-size: 1.1em;
      display: flex;
      align-items: center;
      gap: 0.4em;
    }
    label {
      display: block;
      margin-top: 1em;
      font-size: 0.9em;
      font-weight: 600;
      opacity: 0.85;
    }
    input[type="number"] {
      width: 100%;
      padding: 0.5em;
      margin-top: 0.3em;
      border: 2px solid #ddd;
      border-radius: 10px;
      font-size: 1em;
    }
    input[type="range"] { width: 100%; margin-top: 0.5em; accent-color: var(--accent); }
    .volume-row { display: flex; align-items: center; gap: 0.6em; }
    .volume-emoji { font-size: 1.6em; width: 1.4em; text-align: center; }
    .file-row {
      border: 2px dashed #bbb;
      border-radius: 12px;
      padding: 1em;
      text-align: center;
      margin-top: 0.5em;
      transition: border-color 0.2s;
    }
    .file-row.picked { border-color: var(--accent); }
    #fileName { display: block; margin-top: 0.5em; font-size: 0.85em; opacity: 0.7; }
    button {
      width: 100%;
      margin-top: 1.2em;
      padding: 0.8em;
      border: none;
      border-radius: 12px;
      font-size: 1.05em;
      font-weight: 700;
      color: #fff;
      background: linear-gradient(135deg, var(--accent), var(--accent-dark));
      cursor: pointer;
      transition: transform 0.1s;
    }
    button:active { transform: scale(0.97); }
    #testBtn { background: linear-gradient(135deg, #43e97b, #38f9d7); animation: pulse 2.5s infinite; }
    @keyframes pulse {
      0%, 100% { box-shadow: 0 0 0 0 rgba(67, 233, 123, 0.5); }
      50%      { box-shadow: 0 0 0 10px rgba(67, 233, 123, 0); }
    }
    #toast {
      position: fixed;
      left: 50%;
      bottom: 1.5em;
      transform: translate(-50%, 20px);
      background: #33334d;
      color: #fff;
      padding: 0.8em 1.4em;
      border-radius: 30px;
      font-size: 0.95em;
      opacity: 0;
      pointer-events: none;
      transition: opacity 0.3s, transform 0.3s;
      white-space: nowrap;
    }
    #toast.show { opacity: 1; transform: translate(-50%, 0); }
    .confetti {
      position: fixed;
      top: -2em;
      font-size: 1.6em;
      pointer-events: none;
      animation: fall linear forwards;
      z-index: 999;
    }
    @keyframes fall {
      to { transform: translateY(105vh) rotate(360deg); opacity: 0.3; }
    }
  </style>
</head>
<body>
  <span class="bird">&#128038;</span>
  <h1>SongBird!</h1>
  <p class="subtitle">&#10024; Make your bird sing! &#10024;</p>

  <form class="card" action="/save" method="POST">
    <h2>&#9200; Wake-up time</h2>
    <label>Hour (0-23)
      <input type="number" name="alarmHour" min="0" max="23" value="%ALARM_HOUR%">
    </label>
    <label>Minute (0-59)
      <input type="number" name="alarmMinute" min="0" max="59" value="%ALARM_MINUTE%">
    </label>
    <label>&#128257; How many times should it sing?
      <input type="number" name="loopCount" min="1" max="20" value="%LOOP_COUNT%">
    </label>
    <label>&#128266; How loud?
      <div class="volume-row">
        <span class="volume-emoji" id="volEmoji">&#128266;</span>
        <input type="range" name="volume" id="volSlider" min="0" max="21" value="%VOLUME%"
               oninput="updateVolumeEmoji(this.value)">
      </div>
    </label>
    <button type="submit">&#128190; Save settings</button>
  </form>

  <form class="card" method="POST" action="/upload" enctype="multipart/form-data" id="uploadForm">
    <h2>&#127925; Teach it a new song</h2>
    <div class="file-row" id="fileRow">
      <input type="file" name="song" accept=".mp3" id="fileInput" onchange="fileChosen(this)">
      <span id="fileName">&#128070; Tap to choose an MP3</span>
    </div>
    <button type="submit">&#128228; Upload (replaces current song)</button>
  </form>

  <form class="card" method="POST" action="/test-play">
    <h2>&#9889; Hear it sing right now!</h2>
    <button id="testBtn" type="submit">&#9654;&#65039; Test Play</button>
  </form>

  <div id="toast"></div>

  <script>
    const speakerEmoji = ['&#128263;', '&#128265;', '&#128266;'];
    function updateVolumeEmoji(v) {
      const pct = v / 21;
      const emoji = document.getElementById('volEmoji');
      emoji.textContent = pct === 0 ? '\u{1F507}' : (pct < 0.5 ? '\u{1F509}' : '\u{1F50A}');
    }
    updateVolumeEmoji(document.getElementById('volSlider').value);

    function fileChosen(input) {
      const row = document.getElementById('fileRow');
      const label = document.getElementById('fileName');
      if (input.files.length > 0) {
        row.classList.add('picked');
        label.textContent = '\u{1F3B5} ' + input.files[0].name;
      } else {
        row.classList.remove('picked');
        label.textContent = 'Tap to choose an MP3';
      }
    }

    const CONFETTI_EMOJI = ['\u{1F389}', '\u{1F38A}', '\u{1F3B5}', '\u{1F3B6}', '⭐', '\u{1F425}'];
    function launchConfetti() {
      for (let i = 0; i < 18; i++) {
        const bit = document.createElement('span');
        bit.className = 'confetti';
        bit.textContent = CONFETTI_EMOJI[Math.floor(Math.random() * CONFETTI_EMOJI.length)];
        bit.style.left = Math.random() * 100 + 'vw';
        bit.style.animationDuration = (2 + Math.random() * 1.5) + 's';
        bit.style.fontSize = (1.2 + Math.random() * 1.2) + 'em';
        document.body.appendChild(bit);
        setTimeout(() => bit.remove(), 4000);
      }
    }

    const toastMessages = {
      saved: '✅ Settings saved!',
      uploaded: '\u{1F4E4} New song uploaded!',
      played: '\u{1F3B6} Playing your song...'
    };
    const params = new URLSearchParams(window.location.search);
    for (const key in toastMessages) {
      if (params.has(key)) {
        const toast = document.getElementById('toast');
        toast.textContent = toastMessages[key];
        requestAnimationFrame(() => toast.classList.add('show'));
        setTimeout(() => toast.classList.remove('show'), 2600);
        launchConfetti();
        window.history.replaceState({}, '', window.location.pathname);
        break;
      }
    }
  </script>
</body>
</html>
)HTML";

// We fill in %PLACEHOLDER% values ourselves with plain String::replace()
// rather than ESPAsyncWebServer's built-in template callback: that
// callback naively pairs up every '%' character in the whole page, so
// the literal '%' in our CSS (e.g. "width: 100%") throws off every
// pairing after it and corrupts the rest of the response.
static void handleRoot(AsyncWebServerRequest* request) {
  String html(PAGE_HTML);
  if (g_settings) {
    html.replace("%ALARM_HOUR%", String(g_settings->alarmHour));
    html.replace("%ALARM_MINUTE%", String(g_settings->alarmMinute));
    html.replace("%LOOP_COUNT%", String(g_settings->loopCount));
    html.replace("%VOLUME%", String(g_settings->volume));
  }
  request->send(200, "text/html", html);
}

static void handleSave(AsyncWebServerRequest* request) {
  if (!g_settings) {
    request->send(500, "text/plain", "Settings not available.");
    return;
  }

  if (request->hasParam("alarmHour", true)) {
    g_settings->alarmHour = request->getParam("alarmHour", true)->value().toInt();
  }
  if (request->hasParam("alarmMinute", true)) {
    g_settings->alarmMinute = request->getParam("alarmMinute", true)->value().toInt();
  }
  if (request->hasParam("loopCount", true)) {
    g_settings->loopCount = request->getParam("loopCount", true)->value().toInt();
  }
  if (request->hasParam("volume", true)) {
    g_settings->volume = request->getParam("volume", true)->value().toInt();
  }

  Serial.println("WebModule: settings saved from portal.");
  request->redirect("/?saved=1");
}

static void handleUpload(AsyncWebServerRequest* request, const String& filename,
                          size_t index, uint8_t* data, size_t len, bool final) {
  if (index == 0) {
    Serial.print("WebModule: receiving upload, saving as ");
    Serial.println(SONG_FILENAME);
    g_uploadFile = LittleFS.open(SONG_FILENAME, FILE_WRITE);
  }

  if (g_uploadFile) {
    g_uploadFile.write(data, len);
  }

  if (final) {
    if (g_uploadFile) {
      g_uploadFile.close();
    }
    Serial.println("WebModule: upload complete.");
  }
}

static void handleUploadDone(AsyncWebServerRequest* request) {
  request->redirect("/?uploaded=1");
}

static void handleTestPlay(AsyncWebServerRequest* request) {
  if (g_settings) {
    AudioModule::setVolume(g_settings->volume);
  }
  AudioModule::playSong(SONG_FILENAME);
  Serial.println("WebModule: test-play triggered.");
  request->redirect("/?played=1");
}

void begin(Settings& settings) {
  g_settings = &settings;

  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/upload", HTTP_POST, handleUploadDone, handleUpload);
  server.on("/test-play", HTTP_POST, handleTestPlay);

  server.begin();
  Serial.println("WebModule: portal ready.");
}

void end() {
  server.end();
  g_settings = nullptr;
}

}  // namespace WebModule
