#pragma once

namespace DefaultWeb {

inline constexpr auto kIndexHtml = R"HTML(<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hinge Launcher</title>
    <link rel="icon" type="image/png" href="favicon.png">
    <link rel="stylesheet" href="style.css">
</head>
<body>

<svg class="svg-sprite" aria-hidden="true" focusable="false" xmlns="http://www.w3.org/2000/svg">
    <symbol id="i-brand" viewBox="0 0 24 24"><path d="M5 4v16M19 4v16M5 12h14" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/></symbol>
    <symbol id="i-gamepad" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><rect x="2.5" y="7" width="19" height="10" rx="3.5"/><path d="M7 10.5v3M5.5 12h3"/><path d="M16.2 11.2h.01M18 13h.01"/></g></symbol>
    <symbol id="i-user" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="8" r="3.5"/><path d="M4.5 20c.9-3.6 3.9-5.6 7.5-5.6s6.6 2 7.5 5.6"/></g></symbol>
    <symbol id="i-settings" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.7 1.7 0 0 0 .3 1.9l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.7 1.7 0 0 0-1.9-.3 1.7 1.7 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.7 1.7 0 0 0-1.1-1.5 1.7 1.7 0 0 0-1.9.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.7 1.7 0 0 0 .3-1.9 1.7 1.7 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.7 1.7 0 0 0 1.5-1.1 1.7 1.7 0 0 0-.3-1.9l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.7 1.7 0 0 0 1.9.3H9a1.7 1.7 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.7 1.7 0 0 0 1 1.5 1.7 1.7 0 0 0 1.9-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.7 1.7 0 0 0-.3 1.9V9a1.7 1.7 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.7 1.7 0 0 0-1.5 1Z"/></g></symbol>
    <symbol id="i-terminal" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="5" width="18" height="14" rx="2"/><path d="M7.5 9.5 10 12l-2.5 2.5M12.5 15h4"/></g></symbol>
    <symbol id="i-power" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M12 3v9"/><path d="M6.6 6.6a8 8 0 1 0 10.8 0"/></g></symbol>
    <symbol id="i-download" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M12 3v12"/><path d="m7.5 10.5 4.5 4.5 4.5-4.5"/><path d="M4 20h16"/></g></symbol>
    <symbol id="i-refresh" viewBox="0 0 24 24"><g fill="none" stroke="currentColor" stroke-width="1.6" stroke-linecap="round" stroke-linejoin="round"><path d="M20.5 12a8.5 8.5 0 1 1-2.5-6"/><path d="M20.5 4.5V10h-5.5"/></g></symbol>
    <symbol id="i-play" viewBox="0 0 24 24"><path d="M7 4.5 19 12 7 19.5z" fill="currentColor"/></symbol>
    <symbol id="i-star" viewBox="0 0 24 24"><path d="m12 3.5 2.6 5.6 6 .8-4.4 4.2 1.1 6-5.3-2.9-5.3 2.9 1.1-6L3.4 9.9l6-.8z" fill="none" stroke="currentColor" stroke-width="1.4" stroke-linejoin="round"/></symbol>
    <symbol id="i-star-filled" viewBox="0 0 24 24"><path d="m12 3.5 2.6 5.6 6 .8-4.4 4.2 1.1 6-5.3-2.9-5.3 2.9 1.1-6L3.4 9.9l6-.8z" fill="currentColor"/></symbol>
</svg>

<aside class="sidebar">
    <div class="brand">
        <div class="brand-mark"><svg viewBox="0 0 24 24"><use href="#i-brand"/></svg></div>
        <div class="brand-text">
            <span class="brand-title">HINGE</span>
            <span class="brand-sub">Launcher</span>
        </div>
    </div>

    <nav id="tabs" class="nav-menu">
        <button data-tab="play" class="nav-btn active"><svg class="icon"><use href="#i-gamepad"/></svg><span>Играть</span></button>
        <button data-tab="profiles" class="nav-btn"><svg class="icon"><use href="#i-user"/></svg><span>Профили</span></button>
        <button data-tab="settings" class="nav-btn"><svg class="icon"><use href="#i-settings"/></svg><span>Настройки</span></button>
        <button data-tab="all" class="nav-btn"><svg class="icon"><use href="#i-terminal"/></svg><span>Команды</span></button>
    </nav>

    <div class="sidebar-footer">
        <div class="status-indicator">
            <span id="connDot" class="status-dot"></span>
            <span id="conn">offline</span>
        </div>
        <button id="shutdownBtn" class="icon-btn" title="Выключить лаунчер" aria-label="Выключить лаунчер">
            <svg class="icon"><use href="#i-power"/></svg>
        </button>
    </div>
</aside>

<main class="content-area">

    <header class="top-bar">
        <div class="active-badge">
            <span class="badge-label">Активный профиль</span>
            <span id="activeProfileName" class="badge-value">—</span>
        </div>
        <div class="active-path-box">
            <span class="muted-label">Каталог:</span>
            <span id="activePath" class="path-display" title="">—</span>
        </div>
    </header>

    <div id="installBanner" class="install-banner" style="display:none;">
        <span class="install-banner-dot"></span>
        <span id="installBannerText" class="install-banner-text">—</span>
        <span class="install-banner-spacer"></span>
        <button id="installCancelBtn" class="btn secondary small">Отменить</button>
        <button id="installShowConsole" class="btn secondary small">Показать консоль</button>
    </div>

    <section id="tab-play" class="tab-panel active">
        <div class="play-grid">
            <div class="card version-card">
                <div class="card-header">
                    <h2>Версия Minecraft</h2>
                    <span id="versionCount" class="badge-counter">0</span>
                </div>

                <div class="search-box">
                    <input id="versionSearchTop" class="modern-input" placeholder="Поиск версии: 1.20, 1.12.2…" autocomplete="off">
                </div>

                <div class="chip-filters">
                    <label class="check"><input type="checkbox" id="f-release" checked> Релизы</label>
                    <label class="check"><input type="checkbox" id="f-snapshot"> Снапшоты</label>
                    <label class="check"><input type="checkbox" id="f-old_beta"> Beta</label>
                    <label class="check"><input type="checkbox" id="f-old_alpha"> Alpha</label>
                    <label class="check"><input type="checkbox" id="f-favorites"> Только избранные</label>
                </div>

                <div id="recentBox" class="recent-box" style="display:none;">
                    <div class="recent-label">Недавние</div>
                    <div id="recentList" class="recent-list"></div>
                </div>

                <div class="list-container">
                    <select id="versionList" size="12" class="modern-select"></select>
                </div>

                <div class="fav-row">
                    <button id="favToggle" class="btn secondary small">
                        <svg class="icon"><use href="#i-star"/></svg>
                        <span>В избранное</span>
                    </button>
                </div>
            </div>

            <div class="card config-card">
                <div class="card-header"><h2>Конфигурация запуска</h2></div>

                <div class="field-group">
                    <label class="field-label" for="modloaderType">Модлоадер</label>
                    <select id="modloaderType" class="modern-input">
                        <option value="">Ванилла (без загрузчика)</option>
                        <option value="fabric">Fabric Loader</option>
                        <option value="quilt">Quilt Loader</option>
                        <option value="forge">Minecraft Forge</option>
                        <option value="neoforge">NeoForge</option>
                    </select>
                </div>

                <div id="loaderVersionRow" class="field-group loader-version-group" style="display:none;">
                    <label class="field-label" for="loaderVersion">Версия загрузчика</label>
                    <div class="input-with-action">
                        <input id="loaderVersion" type="text" class="modern-input" placeholder="Версия модлоадера">
                        <button id="fetchLatestBtn" class="btn secondary action-btn" title="Загрузить список версий" aria-label="Загрузить список версий">
                            <svg class="icon"><use href="#i-refresh"/></svg>
                        </button>
                    </div>
                    <select id="loaderVersionsList" size="4" class="modern-select loader-select" style="display:none;"></select>
                </div>

                <div class="launch-actions">
                    <button id="installSelectedBtn" class="btn secondary">
                        <svg class="icon"><use href="#i-download"/></svg>
                        <span>Скачать</span>
                    </button>
                    <button id="playBtn" class="btn primary">
                        <svg class="icon"><use href="#i-play"/></svg>
                        <span>Играть</span>
                    </button>
                </div>

                <div class="hotkey-hint">Ctrl+D — скачать · Ctrl+Enter — играть · Ctrl+K — поиск · ` — консоль</div>
            </div>
        </div>

        <div class="card progress-card">
            <div class="card-header">
                <h2>Прогресс</h2>
                <span id="progressText" class="progress-text muted">Простой.</span>
            </div>
            <div class="progress-track">
                <div id="progressBar" class="progress-fill"></div>
            </div>
            <div class="progress-sub-track">
                <div id="progressSubBar" class="progress-sub-fill"></div>
            </div>
            <div class="progress-meta">
                <span id="progressPhase" class="progress-phase">—</span>
                <span id="progressFile" class="progress-file">—</span>
                <span id="progressSpeed" class="progress-speed">—</span>
            </div>
            <div id="playStatus" class="result"></div>
        </div>
    </section>

    <section id="tab-profiles" class="tab-panel">
        <div class="profiles-grid">
            <div class="card">
                <div class="card-header"><h2>Профили игроков</h2></div>
                <div id="profileList" class="profile-cards-list"></div>

                <div class="create-profile-box">
                    <input id="newProfileName" class="modern-input" placeholder="Имя профиля">
                    <div class="action-row">
                        <button id="createOffline" class="btn secondary">Офлайн-профиль</button>
                        <button id="createMS" class="btn accent-btn">Microsoft-профиль</button>
                    </div>
                </div>
            </div>

            <div class="card">
                <div class="card-header"><h2>Пути установки</h2></div>
                <div id="pathsList" class="paths-list"></div>

                <div class="add-path-box">
                    <input id="newPath" class="modern-input" placeholder="Абсолютный путь к каталогу .minecraft">
                    <div class="memory-input-row">
                        <input id="newPathMem" type="number" class="modern-input mem-input" value="2">
                        <span class="mem-label">GB / MB</span>
                        <button id="addPathBtn" class="btn primary">Добавить путь</button>
                    </div>
                </div>
            </div>

            <div class="card java-card">
                <div class="card-header">
                    <h2>Java Runtime</h2>
                    <span id="javaCount" class="badge-counter">0</span>
                </div>
                <div id="javaList" class="java-list"></div>
                <div class="java-download-row">
                    <span class="muted">Скачать портативную:</span>
                    <button class="btn secondary" data-java-dl="8">Java 8</button>
                    <button class="btn secondary" data-java-dl="17">Java 17</button>
                    <button class="btn secondary" data-java-dl="21">Java 21</button>
                </div>
            </div>
        </div>
    </section>

    <section id="tab-settings" class="tab-panel">
        <div class="card settings-card">
            <div class="card-header"><h2>Параметры лаунчера</h2></div>
            <div id="settingsForm"></div>
        </div>
    </section>

    <section id="tab-all" class="tab-panel">
        <div class="card">
            <div class="card-header">
                <h2>Консоль команд API</h2>
                <label class="check compact-check">
                    <input type="checkbox" id="showAllCommands">
                    <span>Показать все</span>
                </label>
            </div>
            <div id="commands" class="commands-grid grid"></div>
        </div>
    </section>

</main>

<div id="consoleDock" class="console-dock collapsed">
    <div id="consoleResize" class="console-resize"></div>
    <div class="console-header">
        <button id="consoleToggle" class="icon-btn" aria-label="Свернуть консоль">▾</button>
        <span class="console-title">Консоль</span>
        <span id="logCount" class="console-badge">0</span>
        <span id="logErrCount" class="console-badge err">0</span>
        <div class="console-filters">
            <label class="check compact-check"><input type="checkbox" data-lvl="info" checked><span>INFO</span></label>
            <label class="check compact-check"><input type="checkbox" data-lvl="warn" checked><span>WARN</span></label>
            <label class="check compact-check"><input type="checkbox" data-lvl="error" checked><span>ERROR</span></label>
        </div>
        <span class="console-spacer"></span>
        <label class="check compact-check"><input type="checkbox" id="consoleAutoscroll" checked><span>Автоскролл</span></label>
        <button id="consoleCopy" class="btn secondary small">Копировать</button>
        <button id="consoleClear" class="btn secondary small">Очистить</button>
    </div>
    <div id="consoleBody" class="console-body"></div>
</div>

<script src="hinge.js"></script>
<script src="app.js"></script>
<script src="live.js"></script>
</body>
</html>)HTML";

inline constexpr auto kStyleCss = R"CSS(
* { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg:        #0d0f13;
    --bg-soft:   #11141a;
    --panel:     #161a21;
    --panel-hi:  #1b202a;
    --border:    #232935;
    --border-hi: #2e3546;
    --text:      #e4e7ee;
    --text-soft: #b9bec9;
    --muted:     #7a8295;
    --accent:    #4b7bd6;
    --accent-hi: #5d8be6;
    --accent-bg: rgba(75, 123, 214, 0.12);
    --danger:    #d9534f;
    --ok:        #4caa67;
    --warn:      #d9a441;
    --radius:    10px;
    --radius-sm: 6px;
}

html, body { height: 100%; }

body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto,
                 "Helvetica Neue", Arial, sans-serif;
    background: var(--bg);
    color: var(--text);
    font-size: 14px;
    line-height: 1.5;
    display: grid;
    grid-template-columns: 240px 1fr;
    min-height: 100vh;
    -webkit-font-smoothing: antialiased;
}

::selection { background: var(--accent-bg); color: var(--text); }

.svg-sprite { position: absolute; width: 0; height: 0; overflow: hidden; pointer-events: none; }
.icon { width: 18px; height: 18px; display: inline-block; vertical-align: middle; flex-shrink: 0; }

button {
    font-family: inherit;
    cursor: pointer;
    background: var(--panel-hi);
    color: var(--text-soft);
    border: 1px solid var(--border);
    border-radius: var(--radius-sm);
    padding: 6px 12px;
    font-size: 12px;
    transition: background 120ms, color 120ms, border-color 120ms, opacity 120ms;
}
button:hover:not(:disabled) { background: var(--border); color: var(--text); }
button:disabled { opacity: 0.5; cursor: not-allowed; }

aside.sidebar {
    background: var(--bg-soft);
    border-right: 1px solid var(--border);
    padding: 20px 14px 52px;
    display: flex; flex-direction: column; gap: 24px;
    min-height: 100vh;
}

.brand { display: flex; align-items: center; gap: 12px; padding: 4px 6px; }
.brand-mark {
    width: 36px; height: 36px; border-radius: var(--radius-sm);
    background: var(--panel); border: 1px solid var(--border);
    color: var(--accent-hi);
    display: flex; align-items: center; justify-content: center;
}
.brand-mark svg { width: 22px; height: 22px; }
.brand-text { display: flex; flex-direction: column; line-height: 1.1; }
.brand-title { font-size: 15px; font-weight: 700; letter-spacing: 0.08em; }
.brand-sub { font-size: 10px; letter-spacing: 0.12em; color: var(--muted); text-transform: uppercase; margin-top: 2px; }

nav#tabs { display: flex; flex-direction: column; gap: 2px; flex: 1; }
nav#tabs button {
    display: flex; align-items: center; gap: 12px;
    padding: 10px 12px; background: transparent;
    color: var(--muted); border: 0; border-radius: var(--radius-sm);
    cursor: pointer; font-size: 13px; font-family: inherit; text-align: left;
    transition: background 120ms, color 120ms;
}
nav#tabs button:hover { background: var(--panel); color: var(--text-soft); }
nav#tabs button.active { background: var(--panel-hi); color: var(--text); }
nav#tabs button.active .icon { color: var(--accent-hi); }

.sidebar-footer { display: flex; align-items: center; gap: 8px; padding-top: 14px; border-top: 1px solid var(--border); }
.status-indicator { flex: 1; display: flex; align-items: center; gap: 8px; font-size: 12px; color: var(--muted); }
.status-dot {
    width: 8px; height: 8px; border-radius: 50%; background: var(--muted);
    transition: background 200ms, box-shadow 200ms;
}
.status-dot.online  { background: var(--ok);     box-shadow: 0 0 0 3px rgba(76, 170, 103, 0.15); }
.status-dot.offline { background: var(--danger); box-shadow: 0 0 0 3px rgba(217, 83, 79, 0.15); }

.icon-btn {
    display: inline-flex; align-items: center; justify-content: center;
    width: 32px; height: 32px; background: transparent;
    color: var(--muted); border: 1px solid transparent;
    border-radius: var(--radius-sm); cursor: pointer;
    padding: 0;
    transition: color 120ms, background 120ms, border-color 120ms;
}
.icon-btn:hover { color: var(--danger); background: rgba(217, 83, 79, 0.08); border-color: rgba(217, 83, 79, 0.2); }
.icon-btn .icon { width: 16px; height: 16px; }

main.content-area {
    padding: 24px 32px 320px;
    width: 100%; overflow-x: hidden;
}

.top-bar {
    display: flex; align-items: center; gap: 24px;
    padding: 12px 0 20px; border-bottom: 1px solid var(--border);
    margin-bottom: 20px; flex-wrap: wrap;
}
.active-badge {
    display: flex; flex-direction: column; gap: 2px;
    padding: 8px 14px; background: var(--panel);
    border: 1px solid var(--border); border-radius: var(--radius-sm);
    min-width: 180px;
}
.badge-label { font-size: 10px; letter-spacing: 0.1em; text-transform: uppercase; color: var(--muted); }
.badge-value { font-size: 14px; font-weight: 600; color: var(--text); }
.active-path-box { display: flex; align-items: center; gap: 8px; flex: 1; min-width: 0; font-size: 12px; }
.muted-label { color: var(--muted); flex-shrink: 0; }
.path-display {
    color: var(--text-soft);
    font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    font-size: 12px; overflow: hidden; text-overflow: ellipsis; white-space: nowrap;
}

.install-banner {
    display: flex; align-items: center; gap: 12px;
    padding: 10px 14px; margin-bottom: 16px;
    background: var(--accent-bg); border: 1px solid var(--accent);
    border-radius: var(--radius-sm); font-size: 13px;
}
.install-banner-dot {
    width: 8px; height: 8px; border-radius: 50%;
    background: var(--accent-hi);
    animation: pulse 1.2s ease-in-out infinite;
}
@keyframes pulse { 0%,100% { opacity: 1; } 50% { opacity: 0.35; } }
.install-banner-spacer { flex: 1; }

.card {
    background: var(--panel); border: 1px solid var(--border);
    border-radius: var(--radius); padding: 18px; margin-bottom: 16px;
}
.card-header {
    display: flex; align-items: center; justify-content: space-between;
    gap: 12px; margin-bottom: 14px;
}
.card-header h2 { font-size: 14px; font-weight: 600; color: var(--text); letter-spacing: 0.02em; }
.badge-counter {
    display: inline-flex; align-items: center; padding: 2px 8px;
    font-size: 11px; font-weight: 500; background: var(--panel-hi);
    color: var(--muted); border-radius: 20px; border: 1px solid var(--border);
}

.tab-panel { display: none; }
.tab-panel.active { display: block; animation: fade 180ms ease; }
@keyframes fade { from { opacity: 0; } to { opacity: 1; } }

.play-grid, .profiles-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; margin-bottom: 16px; }
.java-card { grid-column: 1 / -1; }
@media (max-width: 900px) { .play-grid, .profiles-grid { grid-template-columns: 1fr; } }

.commands-grid, .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 12px; }

h1 { font-size: 20px; font-weight: 600; }
h3 { font-size: 13px; margin-bottom: 8px; color: var(--text); font-weight: 600; letter-spacing: 0.02em; }
.muted { color: var(--muted); font-size: 12px; }

input, select, .modern-input, .modern-select {
    width: 100%; padding: 9px 12px;
    background: var(--bg-soft); color: var(--text);
    border: 1px solid var(--border); border-radius: var(--radius-sm);
    font-size: 13px; font-family: inherit;
    transition: border-color 120ms, background 120ms;
}
input::placeholder { color: var(--muted); }
input:focus, select:focus { outline: none; border-color: var(--accent); background: var(--panel); }

select[size] {
    font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    font-size: 12px; padding: 6px; line-height: 1.7;
}
select[size] option { padding: 2px 6px; }
select[size] option:checked { background: linear-gradient(var(--accent-bg), var(--accent-bg)); color: var(--text); }

.search-box { margin-bottom: 10px; }
.chip-filters { display: flex; flex-wrap: wrap; gap: 14px; margin-bottom: 12px; }
.list-container { margin-top: 4px; }

.recent-box { margin-bottom: 10px; }
.recent-label { font-size: 10px; letter-spacing: 0.1em; text-transform: uppercase; color: var(--muted); margin-bottom: 6px; }
.recent-list { display: flex; flex-wrap: wrap; gap: 6px; }
.recent-chip {
    padding: 3px 10px; font-size: 11px;
    background: var(--panel-hi); color: var(--text-soft);
    border: 1px solid var(--border); border-radius: 20px;
    cursor: pointer; font-family: inherit;
}
.recent-chip:hover { background: var(--border); color: var(--text); }

.fav-row { display: flex; justify-content: flex-end; margin-top: 8px; }
.fav-row .btn svg { width: 14px; height: 14px; }
.fav-row .btn.active { color: var(--warn); border-color: rgba(217, 164, 65, 0.4); background: rgba(217, 164, 65, 0.08); }

.field-group { margin-bottom: 14px; }
.field-label { display: block; font-size: 11px; letter-spacing: 0.06em; text-transform: uppercase; color: var(--muted); margin-bottom: 6px; }
.input-with-action { display: flex; gap: 8px; }
.input-with-action input { flex: 1; }
.loader-version-group .loader-select { margin-top: 8px; }

.btn {
    display: inline-flex; align-items: center; justify-content: center; gap: 8px;
    padding: 9px 14px; background: var(--panel-hi); color: var(--text-soft);
    border: 1px solid var(--border); border-radius: var(--radius-sm);
    font-size: 13px; font-weight: 500;
}
.btn:hover:not(:disabled) { background: var(--border); color: var(--text); }
.btn.small { padding: 5px 10px; font-size: 11px; }
.btn.primary { background: var(--accent); color: #fff; border-color: var(--accent); }
.btn.primary:hover:not(:disabled) { background: var(--accent-hi); border-color: var(--accent-hi); }
.btn.accent-btn { color: var(--accent-hi); border-color: var(--border-hi); }
.btn.accent-btn:hover:not(:disabled) { background: var(--accent-bg); border-color: var(--accent); }

.action-btn { padding: 9px 12px; flex-shrink: 0; }
.action-btn .icon { width: 16px; height: 16px; }
#fetchLatestBtn.loading .icon { animation: spin 800ms linear infinite; }
@keyframes spin { to { transform: rotate(360deg); } }

.launch-actions { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 18px; padding-top: 18px; border-top: 1px solid var(--border); }
.hotkey-hint { margin-top: 12px; font-size: 10px; color: var(--muted); text-align: center; letter-spacing: 0.04em; }

.action-row { display: flex; gap: 8px; flex-wrap: wrap; }
.create-profile-box { margin-top: 14px; padding-top: 14px; border-top: 1px solid var(--border); }
.create-profile-box .action-row { margin-top: 8px; }

.progress-card .card-header { margin-bottom: 10px; }
.progress-text { font-size: 11px; font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace; }
.progress-track, .progress {
    height: 6px; background: var(--bg-soft); border-radius: 3px;
    overflow: hidden; border: 1px solid var(--border);
}
.progress-sub-track {
    height: 3px; background: var(--bg-soft); border-radius: 2px;
    overflow: hidden; margin-top: 4px;
}
.progress-fill, .progress-bar { height: 100%; background: linear-gradient(90deg, var(--accent), var(--accent-hi)); width: 0; transition: width 220ms ease; }
.progress-sub-fill { height: 100%; background: var(--accent-hi); opacity: 0.7; width: 0; transition: width 120ms linear; }
.progress-meta {
    display: flex; gap: 14px; margin-top: 8px;
    font-size: 11px; color: var(--muted);
    font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
}
.progress-meta .progress-phase { color: var(--text-soft); }
.progress-meta .progress-file { flex: 1; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
.progress-meta .progress-speed { flex-shrink: 0; }

.result {
    margin-top: 10px; padding: 8px 10px;
    font-size: 12px; font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    background: var(--bg-soft); border: 1px solid var(--border);
    border-radius: var(--radius-sm); color: var(--ok);
    white-space: pre-wrap; word-break: break-all;
    max-height: 180px; overflow-y: auto;
}
.result:empty { display: none; }
.result.error { color: var(--danger); border-color: rgba(217, 83, 79, 0.25); }

.profile-cards-list, .paths-list { margin-bottom: 6px; }

.line {
    display: flex; align-items: center; gap: 10px;
    padding: 8px 12px; background: var(--bg-soft);
    border: 1px solid var(--border); border-radius: var(--radius-sm);
    font-size: 13px; margin-bottom: 6px;
    transition: border-color 120ms, background 120ms;
}
.line:hover { border-color: var(--border-hi); }
.line.active { background: var(--accent-bg); border-color: var(--accent); }
.line .name { flex: 1; color: var(--text); }
.line .tag {
    color: var(--muted); font-size: 11px; padding: 2px 8px;
    background: var(--panel-hi); border-radius: 20px;
    border: 1px solid var(--border);
}
.line .path-str {
    flex: 1; font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    font-size: 12px; color: var(--text-soft);
    overflow: hidden; text-overflow: ellipsis; white-space: nowrap;
}

.line.path-line { flex-direction: column; align-items: stretch; gap: 8px; }
.path-row { display: flex; align-items: center; gap: 10px; min-width: 0; }
.path-controls { display: flex; align-items: center; gap: 6px; flex-wrap: wrap; }
.path-controls .spacer { flex: 1; }
.line .java-select {
    width: auto; min-width: 110px; max-width: 170px;
    padding: 4px 8px; font-size: 11px; margin: 0;
}
.java-select.incompatible { border-color: var(--danger); }

.java-list { display: flex; flex-direction: column; gap: 6px; margin-bottom: 14px; }
.java-list:empty { display: none; }
.java-item {
    display: flex; align-items: center; gap: 12px;
    padding: 9px 12px; background: var(--bg-soft);
    border: 1px solid var(--border); border-radius: var(--radius-sm);
    font-size: 13px;
}
.java-item .java-ver {
    font-weight: 600; color: var(--text); min-width: 64px;
    font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    font-size: 12px;
}
.java-item .java-path {
    flex: 1; font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    font-size: 11px; color: var(--muted);
    overflow: hidden; text-overflow: ellipsis; white-space: nowrap;
}
.java-download-row {
    display: flex; align-items: center; gap: 8px;
    padding-top: 14px; border-top: 1px solid var(--border); flex-wrap: wrap;
}

.add-path-box { margin-top: 14px; padding-top: 14px; border-top: 1px solid var(--border); }
.memory-input-row { display: flex; gap: 8px; align-items: center; margin-top: 8px; }
.mem-input { max-width: 90px; }
.mem-label { font-size: 12px; color: var(--muted); white-space: nowrap; flex-shrink: 0; }
.memory-input-row .btn { margin-left: auto; }

.check {
    display: flex; align-items: center; gap: 6px;
    padding: 4px 0; font-size: 13px; cursor: pointer;
    color: var(--text-soft); user-select: none;
}
.check input[type="checkbox"] {
    width: auto; margin: 0; accent-color: var(--accent); cursor: pointer;
}
.compact-check { padding: 0; font-size: 12px; color: var(--muted); gap: 4px; }

.cmd-form { display: flex; flex-direction: column; gap: 12px; margin-bottom: 12px; }
.cmd-field { display: flex; flex-direction: column; gap: 4px; }
.cmd-label { font-size: 11px; letter-spacing: 0.05em; text-transform: uppercase; color: var(--muted); line-height: 1.3; }
.cmd-field input { margin: 0; }
.cmd-bool { padding: 6px 0; color: var(--text-soft); font-size: 13px; }

.settings-card .check { padding: 6px 0; }
.settings-card input:not([type="checkbox"]) { margin-bottom: 8px; }
.settings-card h3 { margin: 8px 0 10px; }
.settings-card button { margin-top: 12px; }

.card p { font-size: 12px; color: var(--muted); margin-bottom: 10px; }

.console-dock {
    position: fixed;
    left: 0; right: 0; bottom: 0;
    background: var(--bg-soft);
    border-top: 1px solid var(--border);
    z-index: 100;
    display: flex; flex-direction: column;
    height: 260px;
    max-height: 80vh;
    min-height: 34px;
    box-shadow: 0 -8px 24px rgba(0, 0, 0, 0.35);
}
.console-dock.collapsed { height: 34px; }
.console-dock.collapsed .console-body,
.console-dock.collapsed .console-resize { display: none; }

.console-resize {
    height: 6px; cursor: ns-resize;
    margin-top: -3px;
    background: transparent;
    flex-shrink: 0;
}
.console-resize:hover { background: var(--accent-bg); }

.console-header {
    display: flex; align-items: center; gap: 12px;
    padding: 6px 14px;
    border-bottom: 1px solid var(--border);
    background: var(--panel);
    min-height: 34px;
    font-size: 12px;
    flex-shrink: 0;
}
.console-title { font-weight: 600; letter-spacing: 0.04em; color: var(--text); }
.console-spacer { flex: 1; }
.console-badge {
    display: inline-flex; align-items: center; justify-content: center;
    min-width: 22px; height: 18px;
    padding: 0 6px;
    font-size: 10px; font-weight: 600;
    background: var(--panel-hi); color: var(--muted);
    border: 1px solid var(--border);
    border-radius: 10px;
    font-family: ui-monospace, monospace;
}
.console-badge.err { color: var(--danger); border-color: rgba(217,83,79,0.3); }
.console-filters { display: flex; gap: 10px; }

#consoleToggle { width: 24px; height: 24px; font-size: 14px; color: var(--muted); padding: 0; }
#consoleToggle:hover { color: var(--text); background: var(--panel-hi); border-color: transparent; }
.console-dock.collapsed #consoleToggle { transform: rotate(180deg); }

.console-body {
    flex: 1; overflow-y: auto;
    padding: 6px 14px;
    font-family: ui-monospace, "SF Mono", Menlo, Consolas, monospace;
    font-size: 11px; line-height: 1.55;
    background: #090b0e;
    color: var(--text-soft);
}
.console-line { white-space: pre-wrap; word-break: break-all; padding: 1px 0; }
.console-line .ts { color: #555b6a; margin-right: 8px; }
.console-line.info .lvl { color: var(--accent-hi); margin-right: 6px; }
.console-line.warn { color: var(--warn); }
.console-line.warn .lvl { color: var(--warn); margin-right: 6px; }
.console-line.error { color: var(--danger); }
.console-line.error .lvl { color: var(--danger); margin-right: 6px; }

::-webkit-scrollbar { width: 8px; height: 8px; }
::-webkit-scrollbar-track { background: var(--bg-soft); }
::-webkit-scrollbar-thumb { background: var(--border-hi); border-radius: 4px; }
)CSS";

inline constexpr auto kHingeJs = R"JS(
const Hinge = {
    async commands() {
        return await fetch('/api/commands').then(r => r.json());
    },
    async invoke(id, params = {}) {
        const r = await fetch('/api/commands/' + id, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(params)
        });
        return await r.json();
    }
};

const LS = {
    get(k, def) {
        try { const v = localStorage.getItem(k); return v === null ? def : JSON.parse(v); }
        catch (_) { return def; }
    },
    set(k, v) { try { localStorage.setItem(k, JSON.stringify(v)); } catch (_) {} },
    del(k) { try { localStorage.removeItem(k); } catch (_) {} }
};
)JS";

inline constexpr auto kAppJs = R"JS(
let allCommands = [];
let profilesPollTimer = null;
let cachedJavas = [];
let cachedVersions = [];
let cachedInstalled = new Set();
let lastInstallState = { busy: false, label: '', cancelling: false };
let lastSnapshotBytes = 0;
let lastSnapshotTime = 0;

const HIDDEN_IN_COMMANDS = new Set([
    'profile.list','profile.current','profile.pick','profile.delete',
    'profile.create.offline','profile.create.ms',
    'profile.addPath','profile.selectPath','profile.removePath',
    'profile.paths','profile.setJava',
    'settings.get','settings.update',
    'install.vanilla','install.fabric','install.quilt',
    'install.forge','install.neoforge','install.cancel','install.lock',
    'modloader.latest','version.list','version.installed','game.launch',
    'java.installed','java.detect','java.download','java.delete',
    'install.status'
]);

let lastSelectedVersion = LS.get('hinge.lastVersion', '');

document.getElementById('modloaderType').onchange = (e) => {
    const row = document.getElementById('loaderVersionRow');
    row.style.display = e.target.value ? 'block' : 'none';
    document.getElementById('loaderVersionsList').style.display = 'none';
    document.getElementById('loaderVersion').value = '';
    LS.set('hinge.modloader', e.target.value);
};

document.getElementById('fetchLatestBtn').onclick = async () => {
    const type = document.getElementById('modloaderType').value;
    const mc = lastSelectedVersion || document.getElementById('versionList').value;
    if (!type || !mc) return;

    const btn = document.getElementById('fetchLatestBtn');
    btn.disabled = true;
    btn.classList.add('loading');

    try {
        const res = await Hinge.invoke('modloader.latest', { type, mc });
        if (!res.ok) { alert('Ошибка: ' + res.error); return; }

        const sel = document.getElementById('loaderVersionsList');
        sel.innerHTML = '';
        sel.style.display = 'block';
        for (const item of res.result.versions) {
            const o = document.createElement('option');
            o.value = item.version;
            const marks = [];
            if (item.stable) marks.push('stable');
            if (item.kind) marks.push(item.kind);
            o.textContent = item.version + (marks.length ? ` (${marks.join(', ')})` : '');
            sel.appendChild(o);
        }
        sel.onchange = () => {
            document.getElementById('loaderVersion').value = sel.value;
            LS.set('hinge.loaderVersion', sel.value);
        };
        if (res.result.versions[0]) {
            document.getElementById('loaderVersion').value = res.result.versions[0].version;
            LS.set('hinge.loaderVersion', res.result.versions[0].version);
        }
    } finally {
        btn.disabled = false;
        btn.classList.remove('loading');
    }
};

document.querySelectorAll('#tabs button').forEach(btn => {
    btn.onclick = () => switchTab(btn.dataset.tab);
});

function switchTab(name) {
    document.querySelectorAll('#tabs button').forEach(b =>
        b.classList.toggle('active', b.dataset.tab === name));
    document.querySelectorAll('.tab-panel').forEach(p =>
        p.classList.toggle('active', p.id === 'tab-' + name));
}

document.getElementById('shutdownBtn').onclick = async () => {
    if (!confirm('Выключить лаунчер?')) return;
    await fetch('/api/shutdown', { method: 'POST' });
    document.body.innerHTML = '<div style="padding:40px;font-family:sans-serif;color:#888">Лаунчер выключен. Вкладку можно закрыть.</div>';
};

function buildForm(command) {
    const wrap = document.createElement('div');
    wrap.className = 'cmd-form';

    for (const p of command.params) {
        if (p.type === 'bool') {
            const row = document.createElement('label');
            row.className = 'check cmd-bool';
            const input = document.createElement('input');
            input.type = 'checkbox';
            input.checked = String(p.default) === 'true';
            input.dataset.param = p.name;
            input.dataset.type = 'bool';
            const text = document.createElement('span');
            text.textContent = p.description || p.name;
            row.appendChild(input);
            row.appendChild(text);
            wrap.appendChild(row);
            continue;
        }

        const field = document.createElement('div');
        field.className = 'cmd-field';
        const label = document.createElement('label');
        label.className = 'cmd-label';
        label.textContent = p.description || p.name;
        field.appendChild(label);

        const input = document.createElement('input');
        input.type = p.type === 'int' ? 'number' : 'text';
        input.value = p.default ?? '';
        input.placeholder = p.name;
        input.dataset.param = p.name;
        input.dataset.type = p.type;
        field.appendChild(input);
        wrap.appendChild(field);
    }
    return wrap;
}

function collectParams(formEl) {
    const out = {};
    formEl.querySelectorAll('[data-param]').forEach(el => {
        const t = el.dataset.type;
        if (t === 'bool')       out[el.dataset.param] = el.checked;
        else if (t === 'int')   out[el.dataset.param] = Number(el.value);
        else                    out[el.dataset.param] = el.value;
    });
    return out;
}

function renderCard(command) {
    const card = document.createElement('div');
    card.className = 'card';
    card.innerHTML = `<h3>${command.title || command.id}</h3>`;
    if (command.description) {
        const p = document.createElement('p');
        p.textContent = command.description;
        card.appendChild(p);
    }
    const form = buildForm(command);
    card.appendChild(form);
    const runBtn = document.createElement('button');
    runBtn.className = 'btn primary';
    runBtn.textContent = 'Выполнить';
    const result = document.createElement('div');
    result.className = 'result';
    runBtn.onclick = async () => {
        result.textContent = '...';
        try {
            const res = await Hinge.invoke(command.id, collectParams(form));
            result.textContent = res.ok ? JSON.stringify(res.result, null, 2) : res.error;
            result.className = 'result' + (res.ok ? '' : ' error');
        } catch (e) {
            result.textContent = e.message;
            result.className = 'result error';
        }
    };
    card.appendChild(runBtn);
    card.appendChild(result);
    return card;
}

function renderCommands() {
    const container = document.getElementById('commands');
    const showAll = document.getElementById('showAllCommands').checked;
    container.innerHTML = '';
    let shown = 0;
    for (const c of allCommands) {
        if (!showAll && HIDDEN_IN_COMMANDS.has(c.id)) continue;
        container.appendChild(renderCard(c));
        shown++;
    }
    if (shown === 0) container.innerHTML = '<div class="muted">Нет доступных команд.</div>';
}

async function refreshJava() {
    const res = await Hinge.invoke('java.installed');
    if (!res.ok) return;
    cachedJavas = res.result.javas || [];

    const list = document.getElementById('javaList');
    list.innerHTML = '';
    if (cachedJavas.length === 0) {
        list.innerHTML = '<div class="muted">Портативные Java не установлены.</div>';
    } else {
        for (const j of cachedJavas) {
            const item = document.createElement('div');
            item.className = 'java-item';

            const v = document.createElement('span');
            v.className = 'java-ver';
            v.textContent = 'Java ' + j.version;
            item.appendChild(v);

            const p = document.createElement('span');
            p.className = 'java-path';
            p.textContent = j.path;
            p.title = j.path;
            item.appendChild(p);

            const del = document.createElement('button');
            del.textContent = 'Удалить';
            del.onclick = async () => {
                if (!confirm('Удалить Java ' + j.version + '?\n' + j.path)) return;
                const r = await Hinge.invoke('java.delete', { version: j.version });
                if (!r.ok) { alert('Ошибка: ' + r.error); return; }
                await refreshJava();
                await refreshProfiles();
            };
            item.appendChild(del);

            list.appendChild(item);
        }
    }
    document.getElementById('javaCount').textContent = cachedJavas.length;
}

document.querySelectorAll('[data-java-dl]').forEach(btn => {
    btn.onclick = async () => {
        const ver = Number(btn.dataset.javaDl);
        const orig = btn.textContent;
        btn.disabled = true;
        btn.textContent = 'Скачивание…';
        try {
            const res = await Hinge.invoke('java.download', { version: ver });
            if (!res.ok) { alert('Ошибка: ' + res.error); return; }
            await refreshJava();
            await refreshProfiles();
        } finally {
            btn.disabled = false;
            btn.textContent = orig;
        }
    };
});

async function refreshProfiles() {
    const listRes = await Hinge.invoke('profile.list');
    if (!listRes.ok) return;
    const list = listRes.result.profiles || [];

    const curRes = await Hinge.invoke('profile.current');
    const active = (curRes.ok && !curRes.result.error) ? curRes.result : null;

    const pathsRes = await Hinge.invoke('profile.paths');
    const paths = pathsRes.ok ? pathsRes.result.paths : [];

    document.getElementById('activeProfileName').textContent = active?.name || '—';
    document.getElementById('activePath').textContent = active?.path || '—';

    const pathsBox = document.getElementById('pathsList');
    pathsBox.innerHTML = '';
    if (paths.length === 0) {
        pathsBox.innerHTML = '<div class="muted">Нет путей. Добавьте ниже.</div>';
    }
    for (const p of paths) {
        const line = document.createElement('div');
        line.className = 'line path-line' + (p.selected ? ' active' : '');

        const topRow = document.createElement('div');
        topRow.className = 'path-row';

        const pathStr = document.createElement('span');
        pathStr.className = 'path-str';
        pathStr.textContent = p.path;
        pathStr.title = p.path;
        topRow.appendChild(pathStr);

        const tag = document.createElement('span');
        tag.className = 'tag';
        tag.textContent = (p.memory ?? '?') + ' GB/MB';
        topRow.appendChild(tag);

        line.appendChild(topRow);

        const ctrl = document.createElement('div');
        ctrl.className = 'path-controls';

        const javaSel = document.createElement('select');
        javaSel.className = 'java-select';
        javaSel.title = 'Java для этого пути';

        const autoOpt = document.createElement('option');
        autoOpt.value = '';
        autoOpt.textContent = 'Java: Авто';
        javaSel.appendChild(autoOpt);

        for (const j of cachedJavas) {
            const opt = document.createElement('option');
            opt.value = j.path;
            opt.textContent = 'Java ' + j.version;
            opt.title = j.path;
            javaSel.appendChild(opt);
        }
        javaSel.value = p.java || '';
        javaSel.onchange = async () => {
            await Hinge.invoke('profile.setJava', { path: p.path, java: javaSel.value });
        };
        ctrl.appendChild(javaSel);

        const autoBtn = document.createElement('button');
        autoBtn.textContent = 'Авто';
        autoBtn.title = 'Определить необходимую Java автоматически';
        autoBtn.onclick = async () => {
            const ver = lastSelectedVersion || '';
            if (!ver) { alert('Сначала выберите версию Minecraft'); return; }
            const det = await Hinge.invoke('java.detect', { version: ver });
            if (!det.ok) { alert('Ошибка: ' + det.error); return; }
            const need = det.result.requiredMajor;
            const match = cachedJavas.find(j => Number(j.version) === need);
            if (!match) {
                alert('Java ' + need + ' не установлена. Скачайте её в блоке Java Runtime.');
                return;
            }
            await Hinge.invoke('profile.setJava', { path: p.path, java: match.path });
            await refreshProfiles();
        };
        ctrl.appendChild(autoBtn);

        const spacer = document.createElement('span');
        spacer.className = 'spacer';
        ctrl.appendChild(spacer);

        if (!p.selected) {
            const sel = document.createElement('button');
            sel.textContent = 'Выбрать';
            sel.onclick = async () => {
                await Hinge.invoke('profile.selectPath', { path: p.path });
                refreshProfiles();
            };
            ctrl.appendChild(sel);
        }

        const del = document.createElement('button');
        del.textContent = 'Удалить';
        del.onclick = async () => {
            if (!confirm('Удалить путь?\n' + p.path)) return;
            await Hinge.invoke('profile.removePath', { path: p.path });
            refreshProfiles();
        };
        ctrl.appendChild(del);

        line.appendChild(ctrl);
        pathsBox.appendChild(line);
    }

    const profBox = document.getElementById('profileList');
    profBox.innerHTML = '';
    for (const p of list) {
        const line = document.createElement('div');
        line.className = 'line' + (p.name === active?.name ? ' active' : '');

        const name = document.createElement('span');
        name.className = 'name';
        name.textContent = p.name;
        line.appendChild(name);

        const tag = document.createElement('span');
        tag.className = 'tag';
        tag.textContent = p.unlicensed ? 'offline' : 'Microsoft';
        line.appendChild(tag);

        if (p.name !== active?.name) {
            const pick = document.createElement('button');
            pick.textContent = 'Выбрать';
            pick.onclick = async () => {
                await Hinge.invoke('profile.pick', { name: p.name });
                refreshProfiles();
                refreshVersions();
            };
            line.appendChild(pick);
        }

        const del = document.createElement('button');
        del.textContent = 'Удалить';
        del.onclick = async () => {
            if (!confirm('Удалить профиль ' + p.name + '?')) return;
            await Hinge.invoke('profile.delete', { name: p.name });
            refreshProfiles();
        };
        line.appendChild(del);

        profBox.appendChild(line);
    }
}

document.getElementById('addPathBtn').onclick = async () => {
    const path = document.getElementById('newPath').value.trim();
    const memory = Number(document.getElementById('newPathMem').value) || 2;
    if (!path) return;
    const res = await Hinge.invoke('profile.addPath', { path, memory });
    if (!res.ok) { alert('Ошибка: ' + res.error); return; }
    document.getElementById('newPath').value = '';
    refreshProfiles();
};

document.getElementById('createOffline').onclick = async () => {
    const name = document.getElementById('newProfileName').value.trim();
    if (!name) return;
    const res = await Hinge.invoke('profile.create.offline', { name });
    if (!res.ok) { alert('Ошибка: ' + res.error); return; }
    document.getElementById('newProfileName').value = '';
    refreshProfiles();
};

document.getElementById('createMS').onclick = async () => {
    const res = await Hinge.invoke('profile.create.ms', { usePrism: false });
    if (!res.ok) { alert('Ошибка: ' + res.error); return; }
    alert('Открыт браузер. Профиль появится здесь через 5–30 секунд.');
};

function favorites() { return LS.get('hinge.favorites', []); }
function setFavorites(v) { LS.set('hinge.favorites', v); }
function recentlyPlayed() { return LS.get('hinge.recent', []); }
function pushRecent(v) {
    const arr = recentlyPlayed().filter(x => x !== v);
    arr.unshift(v);
    LS.set('hinge.recent', arr.slice(0, 5));
    renderRecent();
}

function renderRecent() {
    const box = document.getElementById('recentBox');
    const list = document.getElementById('recentList');
    const arr = recentlyPlayed();
    if (arr.length === 0) { box.style.display = 'none'; return; }
    box.style.display = 'block';
    list.innerHTML = '';
    for (const v of arr) {
        const btn = document.createElement('button');
        btn.className = 'recent-chip';
        btn.textContent = v;
        btn.onclick = () => {
            lastSelectedVersion = v;
            LS.set('hinge.lastVersion', v);
            renderVersionList(true);
            document.getElementById('versionSearchTop').value = '';
            updateFavButton();
        };
        list.appendChild(btn);
    }
}

function updateFavButton() {
    const btn = document.getElementById('favToggle');
    const span = btn.querySelector('span');
    const use = btn.querySelector('use');
    const v = lastSelectedVersion;
    const favs = favorites();
    if (!v || !favs.includes(v)) {
        span.textContent = 'В избранное';
        use.setAttribute('href', '#i-star');
        btn.classList.remove('active');
    } else {
        span.textContent = 'В избранном';
        use.setAttribute('href', '#i-star-filled');
        btn.classList.add('active');
    }
}

document.getElementById('favToggle').onclick = () => {
    const v = lastSelectedVersion;
    if (!v) return;
    let favs = favorites();
    if (favs.includes(v)) favs = favs.filter(x => x !== v);
    else favs.push(v);
    setFavorites(favs);
    updateFavButton();
    renderVersionList(true);
};

async function refreshVersions() {
    const filter = {
        release:   document.getElementById('f-release').checked,
        snapshot:  document.getElementById('f-snapshot').checked,
        old_beta:  document.getElementById('f-old_beta').checked,
        old_alpha: document.getElementById('f-old_alpha').checked
    };
    LS.set('hinge.filters', filter);

    const sel = document.getElementById('versionList');
    sel.innerHTML = '<option disabled>Загрузка...</option>';

    const [listRes, instRes] = await Promise.all([
        Hinge.invoke('version.list', filter),
        Hinge.invoke('version.installed')
    ]);

    if (!listRes.ok) {
        sel.innerHTML = '<option disabled>' + (listRes.result?.error || 'Ошибка') + '</option>';
        return;
    }
    cachedVersions = listRes.result.versions || [];
    cachedInstalled = new Set(instRes.ok ? instRes.result.versions || [] : []);

    renderVersionList(true);
}

function renderVersionList(preserve) {
    const sel = document.getElementById('versionList');
    const q = (document.getElementById('versionSearchTop').value || '').toLowerCase().trim();
    const favOnly = document.getElementById('f-favorites').checked;
    const favs = favorites();

    sel.innerHTML = '';
    let shown = 0;

    for (const v of cachedVersions) {
        if (favOnly && !favs.includes(v.id)) continue;
        if (q && !v.id.toLowerCase().includes(q)) continue;

        const opt = document.createElement('option');
        opt.value = v.id;
        const installed = cachedInstalled.has(v.id) ? '\u25CF ' : '   ';
        const star = favs.includes(v.id) ? '\u2605 ' : '';
        opt.textContent = `${installed}${star}${v.id} (${v.type})`;
        sel.appendChild(opt);
        if (++shown >= 300) break;
    }

    if (lastSelectedVersion) {
        for (const opt of sel.options) {
            if (opt.value === lastSelectedVersion) { opt.selected = true; break; }
        }
    }
    if (!sel.value && sel.options.length) {
        lastSelectedVersion = sel.options[0].value;
        LS.set('hinge.lastVersion', lastSelectedVersion);
    }

    document.getElementById('versionCount').textContent = `(${shown}/${cachedVersions.length})`;
    updateFavButton();
}

document.getElementById('versionList').onchange = (e) => {
    lastSelectedVersion = e.target.value;
    LS.set('hinge.lastVersion', lastSelectedVersion);
    updateFavButton();
};

document.getElementById('versionSearchTop').oninput = () => renderVersionList(false);
document.getElementById('f-favorites').onchange = () => renderVersionList(false);

['f-release','f-snapshot','f-old_beta','f-old_alpha'].forEach(id => {
    document.getElementById(id).onchange = refreshVersions;
});

function fmtBytes(n) {
    if (!n || n < 0) return '0 B';
    const units = ['B','KB','MB','GB','TB'];
    let i = 0; let v = n;
    while (v >= 1024 && i < units.length - 1) { v /= 1024; i++; }
    return v.toFixed(v < 10 && i > 0 ? 1 : 0) + ' ' + units[i];
}

function fmtSpeed(bps) {
    if (!bps || bps <= 0) return '';
    return fmtBytes(bps) + '/с';
}

function fmtEta(sec) {
    if (!sec || sec < 0 || sec > 86400) return '';
    if (sec < 60) return sec + ' сек';
    if (sec < 3600) return Math.floor(sec / 60) + ' мин';
    return Math.floor(sec / 3600) + ' ч ' + Math.floor((sec % 3600) / 60) + ' мин';
}

function updateProgressUI(s) {
    const overall = Math.max(0, Math.min(100, s.readyPercent || 0));
    document.getElementById('progressBar').style.width = overall + '%';

    const sub = Math.max(0, Math.min(100, s.currentFilePercent || 0));
    document.getElementById('progressSubBar').style.width = sub + '%';

    let text = (s.downloadedCount || 0) + ' / ' + (s.totalFiles || 0);
    if (s.cancelling) text = 'Отмена… ' + text;
    document.getElementById('progressText').textContent = text;

    document.getElementById('progressPhase').textContent = s.phase || '—';
    document.getElementById('progressFile').textContent = s.detail || s.file || '—';

    const now = Date.now();
    let speedTxt = '';
    if (s.totalBytes > 0) {
        const db = s.downloadedBytes - lastSnapshotBytes;
        const dt = (now - lastSnapshotTime) / 1000;
        if (dt > 0.2 && db > 0) {
            const bps = db / dt;
            speedTxt = fmtSpeed(bps);
            const remain = s.totalBytes - s.downloadedBytes;
            if (remain > 0 && bps > 0) {
                const eta = Math.round(remain / bps);
                if (eta < 86400) speedTxt += ' · ETA ' + fmtEta(eta);
            }
        }
        speedTxt += ' · ' + fmtBytes(s.downloadedBytes) + ' / ' + fmtBytes(s.totalBytes);
    }
    document.getElementById('progressSpeed').textContent = speedTxt;

    lastSnapshotBytes = s.downloadedBytes || 0;
    lastSnapshotTime = now;
}

function lockUI(busy, label) {
    const ids = ['installSelectedBtn','playBtn','fetchLatestBtn','addPathBtn',
                 'createOffline','createMS','shutdownBtn'];
    for (const id of ids) {
        const el = document.getElementById(id);
        if (el) el.disabled = busy;
    }
    document.querySelectorAll('[data-java-dl]').forEach(b => b.disabled = busy);
    document.querySelectorAll('.line button').forEach(b => {
        if (b.textContent === 'Выбрать' || b.textContent === 'Удалить' || b.textContent === 'Авто')
            b.disabled = busy;
    });

    const banner = document.getElementById('installBanner');
    if (busy) {
        banner.style.display = 'flex';
        document.getElementById('installBannerText').textContent =
            'Идёт установка: ' + (label || 'неизвестно');
    } else {
        banner.style.display = 'none';
    }
}

async function pollInstallStatus() {
    const [st, lk] = await Promise.all([
        Hinge.invoke('install.status'),
        Hinge.invoke('install.lock')
    ]);
    if (!st.ok) return;

    const s = st.result;
    const busy = !!(lk.ok && lk.result.busy);
    updateProgressUI(s);

    if (busy !== lastInstallState.busy) {
        lockUI(busy, lk.ok ? lk.result.label : '');
        if (!busy) {
            await refreshVersions();
            await refreshProfiles();
            if (document.hidden) notify('Установка завершена', s.phase || '');
        }
        lastInstallState = lk.ok ? lk.result : { busy, label: '', cancelling: false };
    }
}

async function waitForLockFree() {
    await new Promise(r => setTimeout(r, 200));
    while (true) {
        const lk = await Hinge.invoke('install.lock');
        if (lk.ok && !lk.result.busy) return;
        await new Promise(r => setTimeout(r, 300));
    }
}

document.getElementById('installSelectedBtn').onclick = () => {
    const version = lastSelectedVersion || document.getElementById('versionList').value;
    const mlType = document.getElementById('modloaderType').value;
    const mlVer = document.getElementById('loaderVersion').value.trim();
    if (!version) return;

    const status = document.getElementById('playStatus');
    status.className = 'result';
    status.textContent = 'Запуск установки Minecraft ' + version + '…';

    (async () => {
        const r1 = await Hinge.invoke('install.vanilla', { version });
        if (!r1.ok) {
            status.className = 'result error';
            status.textContent = 'Ошибка: ' + r1.error;
            return;
        }
        await waitForLockFree();

        if (mlType && mlVer) {
            status.textContent = 'Minecraft ' + version + ' установлен. Ставлю ' +
                                 mlType + ' ' + mlVer + '…';
            const params = (mlType === 'neoforge')
                ? { loader: mlVer }
                : { mc: version, loader: mlVer };
            const r2 = await Hinge.invoke('install.' + mlType, params);
            if (!r2.ok) {
                status.className = 'result error';
                status.textContent = 'Ошибка: ' + r2.error;
                return;
            }
            await waitForLockFree();
            status.textContent = 'Установлено: Minecraft ' + version + ' + ' + mlType + ' ' + mlVer;
        } else {
            status.textContent = 'Установлено: Minecraft ' + version;
        }
    })();
};

document.getElementById('playBtn').onclick = async () => {
    const version = lastSelectedVersion || document.getElementById('versionList').value;
    const mlType = document.getElementById('modloaderType').value;
    const mlVer = document.getElementById('loaderVersion').value.trim();
    if (!version) return;

    let modloaderId = '';
    if (mlType && mlVer) {
        if (mlType === 'fabric')        modloaderId = `fabric-loader-${mlVer}-${version}`;
        else if (mlType === 'quilt')    modloaderId = `quilt-loader-${mlVer}-${version}`;
        else if (mlType === 'forge')    modloaderId = `${version}-forge-${mlVer}`;
        else if (mlType === 'neoforge') {
            if (mlVer.startsWith(version + '-')) modloaderId = `${version}-forge-${mlVer.slice(version.length + 1)}`;
            else modloaderId = `neoforge-${mlVer}`;
        }
    }

    const status = document.getElementById('playStatus');
    status.className = 'result';
    status.textContent = 'Запуск...';
    const res = await Hinge.invoke('game.launch', { version, modloader: modloaderId });
    status.className = 'result' + (res.ok ? '' : ' error');
    status.textContent = res.ok
        ? `Запущено: ${version}${mlType ? ' + ' + mlType : ''}`
        : res.error;
    if (res.ok) pushRecent(version);
};

document.getElementById('installCancelBtn').onclick = async () => {
    await Hinge.invoke('install.cancel');
    document.getElementById('installBannerText').textContent = 'Запрошена отмена, ожидание…';
};

document.getElementById('installShowConsole').onclick = () => setConsoleOpen(true);

async function renderSettings() {
    const res = await Hinge.invoke('settings.get');
    if (!res.ok) return;
    const s = res.result;
    const el = document.getElementById('settingsForm');
    el.innerHTML = '<h3>Основные</h3>';

    const mk = (key, type, label, val, hint) => {
        const wrap = document.createElement('div');
        if (type === 'bool') {
            const lbl = document.createElement('label');
            lbl.className = 'check';
            const inp = document.createElement('input');
            inp.type = 'checkbox';
            inp.checked = !!val;
            inp.dataset.key = key;
            inp.dataset.type = 'bool';
            lbl.appendChild(inp);
            lbl.appendChild(document.createTextNode(' ' + label));
            wrap.appendChild(lbl);
        } else {
            const inp = document.createElement('input');
            inp.dataset.key = key;
            inp.dataset.type = type;
            inp.placeholder = label;
            inp.value = val;
            if (hint) inp.title = hint;
            wrap.appendChild(inp);
        }
        if (hint && type === 'bool') {
            const h = document.createElement('div');
            h.className = 'muted';
            h.style.marginBottom = '8px';
            h.textContent = hint;
            wrap.appendChild(h);
        }
        el.appendChild(wrap);
    };

    mk('port', 'int', 'Порт', s.port, '-1 = случайный. Применится при следующем запуске.');
    mk('closeWithSite', 'bool', 'Закрывать лаунчер при закрытии вкладки', s.closeWithSite, 'Работает, если вкладку закрыть (не свернуть).');
    mk('autoOpenBrowser', 'bool', 'Открывать браузер при старте', s.autoOpenBrowser);

    const btn = document.createElement('button');
    btn.className = 'btn primary';
    btn.textContent = 'Сохранить';
    btn.onclick = async () => {
        const params = {};
        el.querySelectorAll('[data-key]').forEach(i => {
            if (i.dataset.type === 'bool')       params[i.dataset.key] = i.checked;
            else if (i.dataset.type === 'int')   params[i.dataset.key] = Number(i.value);
            else                                  params[i.dataset.key] = i.value;
        });
        const r = await Hinge.invoke('settings.update', params);
        alert(r.ok ? 'Сохранено' : 'Ошибка: ' + r.error);
        if (params.closeWithSite !== undefined) {
            if (params.closeWithSite) installBeforeUnload();
            else window.removeEventListener('beforeunload', beforeUnloadHandler);
        }
    };
    el.appendChild(btn);
}

let lastLogSeq = 0;
let logCount = 0;
let logErrCount = 0;
let logAutoscroll = true;
const logFilters = { info: true, warn: true, error: true };

function setConsoleOpen(open) {
    const dock = document.getElementById('consoleDock');
    dock.classList.toggle('collapsed', !open);
    if (open) {
        const h = LS.get('hinge.console.height', 260);
        if (h > 60) dock.style.height = h + 'px';
        scrollConsoleBottom();
    }
    LS.set('hinge.console.open', open);
}

function scrollConsoleBottom() {
    if (!logAutoscroll) return;
    const body = document.getElementById('consoleBody');
    body.scrollTop = body.scrollHeight;
}

function appendLog(entry) {
    if (!logFilters[entry.level]) return;
    const body = document.getElementById('consoleBody');
    const div = document.createElement('div');
    div.className = 'console-line ' + entry.level;
    const d = new Date(entry.ts);
    const hh = String(d.getHours()).padStart(2, '0');
    const mm = String(d.getMinutes()).padStart(2, '0');
    const ss = String(d.getSeconds()).padStart(2, '0');
    const lvl = entry.level === 'warn' ? 'WRN' : entry.level === 'error' ? 'ERR' : 'INF';
    div.innerHTML = `<span class="ts">${hh}:${mm}:${ss}</span><span class="lvl">${lvl}</span>`;
    div.appendChild(document.createTextNode(entry.message));

    body.appendChild(div);
    while (body.childElementCount > 800) body.removeChild(body.firstChild);
    scrollConsoleBottom();
}

async function pollLogs() {
    try {
        const r = await fetch('/api/logs?since=' + lastLogSeq, { cache: 'no-store' });
        const j = await r.json();
        for (const e of j.entries || []) {
            appendLog(e);
            logCount++;
            if (e.level === 'error') logErrCount++;
        }
        lastLogSeq = j.lastSeq || lastLogSeq;
        document.getElementById('logCount').textContent = logCount;
        document.getElementById('logErrCount').textContent = logErrCount;
    } catch (_) {}
}

document.getElementById('consoleToggle').onclick = () => {
    const dock = document.getElementById('consoleDock');
    setConsoleOpen(dock.classList.contains('collapsed'));
};

document.getElementById('consoleClear').onclick = async () => {
    await fetch('/api/logs/clear', { method: 'POST' });
    document.getElementById('consoleBody').innerHTML = '';
    logCount = 0; logErrCount = 0;
    document.getElementById('logCount').textContent = '0';
    document.getElementById('logErrCount').textContent = '0';
};

document.getElementById('consoleCopy').onclick = async () => {
    const body = document.getElementById('consoleBody');
    const text = Array.from(body.querySelectorAll('.console-line'))
        .map(el => el.textContent).join('\n');
    try { await navigator.clipboard.writeText(text); } catch (_) {}
};

document.getElementById('consoleAutoscroll').onchange = (e) => {
    logAutoscroll = e.target.checked;
    scrollConsoleBottom();
};

document.querySelectorAll('.console-filters input').forEach(chk => {
    chk.onchange = () => {
        logFilters[chk.dataset.lvl] = chk.checked;
        const body = document.getElementById('consoleBody');
        body.querySelectorAll('.console-line').forEach(line => {
            const cls = line.className.split(' ')[1];
            line.style.display = logFilters[cls] ? '' : 'none';
        });
    };
});

(function initConsoleResize() {
    const dock = document.getElementById('consoleDock');
    const handle = document.getElementById('consoleResize');
    let startY = 0, startH = 0, resizing = false;

    handle.addEventListener('mousedown', (e) => {
        resizing = true;
        startY = e.clientY;
        startH = dock.getBoundingClientRect().height;
        document.body.style.userSelect = 'none';
        e.preventDefault();
    });

    window.addEventListener('mousemove', (e) => {
        if (!resizing) return;
        const delta = startY - e.clientY;
        const h = Math.max(80, Math.min(window.innerHeight - 100, startH + delta));
        dock.style.height = h + 'px';
        dock.classList.remove('collapsed');
    });

    window.addEventListener('mouseup', () => {
        if (!resizing) return;
        resizing = false;
        document.body.style.userSelect = '';
        LS.set('hinge.console.height', dock.getBoundingClientRect().height);
        LS.set('hinge.console.open', !dock.classList.contains('collapsed'));
    });
})();

function notify(title, body) {
    if (!('Notification' in window)) return;
    if (Notification.permission === 'default') Notification.requestPermission();
    if (Notification.permission === 'granted') {
        try { new Notification(title, { body }); } catch (_) {}
    }
}

document.addEventListener('keydown', (e) => {
    if (e.target.matches('input, select, textarea')) {
        if (e.key === '`' && e.target.id === 'versionSearchTop') e.target.blur();
        return;
    }
    if (e.ctrlKey && e.key === 'Enter') { e.preventDefault(); document.getElementById('playBtn').click(); }
    else if (e.ctrlKey && (e.key === 'd' || e.key === 'D' || e.key === 'в')) { e.preventDefault(); document.getElementById('installSelectedBtn').click(); }
    else if (e.ctrlKey && (e.key === 'k' || e.key === 'K' || e.key === 'л')) { e.preventDefault(); document.getElementById('versionSearchTop').focus(); }
    else if (e.key === '`' || e.key === 'ё') {
        e.preventDefault();
        const dock = document.getElementById('consoleDock');
        setConsoleOpen(dock.classList.contains('collapsed'));
    }
});

function beforeUnloadHandler() {
    navigator.sendBeacon('/api/shutdown', new Blob(['{}'], { type: 'application/json' }));
}
function installBeforeUnload() {
    window.removeEventListener('beforeunload', beforeUnloadHandler);
    window.addEventListener('beforeunload', beforeUnloadHandler);
}

async function init() {
    const connEl = document.getElementById('conn');
    const dotEl = document.getElementById('connDot');

    try {
        allCommands = await Hinge.commands();
        connEl.textContent = 'online';
        dotEl.className = 'status-dot online';
    } catch (e) {
        connEl.textContent = 'offline';
        dotEl.className = 'status-dot offline';
        return;
    }

    document.getElementById('showAllCommands').onchange = renderCommands;
    renderCommands();

    const savedFilters = LS.get('hinge.filters', null);
    if (savedFilters) {
        document.getElementById('f-release').checked = !!savedFilters.release;
        document.getElementById('f-snapshot').checked = !!savedFilters.snapshot;
        document.getElementById('f-old_beta').checked = !!savedFilters.old_beta;
        document.getElementById('f-old_alpha').checked = !!savedFilters.old_alpha;
    }
    const savedMl = LS.get('hinge.modloader', '');
    if (savedMl) {
        document.getElementById('modloaderType').value = savedMl;
        document.getElementById('modloaderType').onchange({ target: { value: savedMl } });
        const savedVer = LS.get('hinge.loaderVersion', '');
        if (savedVer) document.getElementById('loaderVersion').value = savedVer;
    }
    document.getElementById('versionSearchTop').value = LS.get('hinge.search', '');
    document.getElementById('versionSearchTop').addEventListener('input', (e) => {
        LS.set('hinge.search', e.target.value);
    });

    renderRecent();

    const consoleOpen = LS.get('hinge.console.open', false);
    setConsoleOpen(consoleOpen);

    await refreshJava();
    await refreshProfiles();
    await renderSettings();
    await refreshVersions();
    await pollInstallStatus();
    await pollLogs();

    setInterval(pollLogs, 700);
    setInterval(pollInstallStatus, 600);
    profilesPollTimer = setInterval(refreshProfiles, 2500);

    const s = await Hinge.invoke('settings.get');
    if (s.ok && s.result.closeWithSite) installBeforeUnload();
}

init();
)JS";

inline constexpr auto kLiveJs = R"JS(
(() => {
    let known = null;
    setInterval(async () => {
        try {
            const r = await fetch('/api/web-version', { cache: 'no-store' });
            const { version } = await r.json();
            if (known === null) known = version;
            else if (version !== known) location.reload();
        } catch (_) {}
    }, 1000);
})();
)JS";

}
