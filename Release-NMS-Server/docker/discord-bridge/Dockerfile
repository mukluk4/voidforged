# OOC <-> Discord bridge. Talks to world's telnet console over the compose network; see bridge.py.
FROM python:3.12-slim
RUN pip install --no-cache-dir discord.py==2.4.0
COPY bridge.py /app/bridge.py
USER nobody
CMD ["python", "-u", "/app/bridge.py"]
