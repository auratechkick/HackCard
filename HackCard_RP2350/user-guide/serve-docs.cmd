@echo off
cd /d "%~dp0"
echo Starting HackCard RP2350 user guide at http://127.0.0.1:8001/HackCard/rp2350/
echo Leave this window open. Press Ctrl+C to stop.
python -m mkdocs serve -a 127.0.0.1:8001
pause
