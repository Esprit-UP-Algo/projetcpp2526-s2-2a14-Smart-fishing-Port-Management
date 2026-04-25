import cv2
import sys
import os
import json
import sqlite3
import numpy as np
import io
import time
from deepface import DeepFace

# Force UTF-8, unbuffered output so Qt QProcess reads it immediately
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', write_through=True)
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8', write_through=True)

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(SCRIPT_DIR, "faces.db")

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL,
            role TEXT NOT NULL,
            embedding BLOB NOT NULL
        )
    ''')
    conn.commit()
    conn.close()

def get_db_users():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("SELECT id, name, role, embedding FROM users")
    rows = cursor.fetchall()
    conn.close()
    users = []
    for row in rows:
        user_id, name, role, embedding_blob = row
        stored_embedding = np.frombuffer(embedding_blob)
        users.append({"id": user_id, "name": name, "role": role, "embedding": stored_embedding})
    return users

def get_face_embedding(frame):
    try:
        results = DeepFace.represent(frame, model_name="Facenet", enforce_detection=False)
        if results and "embedding" in results[0]:
            return np.array(results[0]["embedding"])
    except:
        pass
    return None

def register_user(name, role):
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute("SELECT id FROM users WHERE name = ?", (name,))
    if cursor.fetchone():
        print(f"ERROR: Face ID for {name} is already registered!", flush=True)
        conn.close()
        return
    conn.close()

    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("ERROR: Could not open webcam.", flush=True)
        return

    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

    # Pre-warm the model
    try:
        DeepFace.build_model("Facenet")
    except:
        pass

    start_time = time.time()
    while time.time() - start_time < 20:
        ret, frame = cap.read()
        if not ret:
            break
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = face_cascade.detectMultiScale(gray, 1.1, 5, minSize=(100, 100))
        for (x, y, w, h) in faces:
            face_roi = frame[max(0,y-20):y+h+20, max(0,x-20):x+w+20]
            if face_roi.size == 0:
                continue
            embedding = get_face_embedding(face_roi)
            if embedding is None:
                continue

            users = get_db_users()
            for u in users:
                dist = np.dot(embedding, u["embedding"]) / (np.linalg.norm(embedding) * np.linalg.norm(u["embedding"]))
                if dist > 0.7:
                    print("ERROR: This face is already registered to another user!", flush=True)
                    cap.release()
                    return

            conn = sqlite3.connect(DB_PATH)
            cursor = conn.cursor()
            try:
                cursor.execute("INSERT INTO users (name, role, embedding) VALUES (?, ?, ?)",
                               (name, role, embedding.tobytes()))
                conn.commit()
                print(f"SUCCESS: User {name} registered as {role}.", flush=True)
            except Exception as e:
                print(f"ERROR: {str(e)}", flush=True)
            finally:
                conn.close()
            cap.release()
            return

    print("ERROR: Registration timed out. No face detected.", flush=True)
    cap.release()


def verify_user_prewarmed():
    """
    Pre-warm mode: load everything up front, print READY, then
    wait for a GO signal on stdin before starting to scan.
    This makes the button click feel instant.
    """
    users = get_db_users()
    if not users:
        print(json.dumps({"status": "failed", "message": "No users registered in database."}), flush=True)
        return

    # Open camera early — this alone saves 1-2 seconds
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print(json.dumps({"status": "error", "message": "Could not open webcam"}), flush=True)
        return

    face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

    # Pre-load DeepFace Facenet model into memory
    try:
        DeepFace.build_model("Facenet")
    except:
        pass

    # Signal Qt that we are warmed up and waiting
    print("READY", flush=True)

    # Block until Qt sends "GO\n" via stdin
    try:
        signal = sys.stdin.readline().strip()
        if signal != "GO":
            cap.release()
            print(json.dumps({"status": "failed", "message": "Cancelled"}), flush=True)
            return
    except:
        cap.release()
        return

    # --- Now scan as fast as possible ---
    start_time = time.time()
    frame_count = 0
    matched = False

    while time.time() - start_time < 12:
        ret, frame = cap.read()
        if not ret:
            break

        frame_count += 1
        # Process every 2nd frame instead of 3rd for better speed
        if frame_count % 2 != 0:
            continue

        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        # Use half-resolution for face detection (fast)
        small = cv2.resize(gray, (0, 0), fx=0.5, fy=0.5)
        faces = face_cascade.detectMultiScale(small, 1.1, 4, minSize=(40, 40))

        for (x, y, w, h) in faces:
            x, y, w, h = int(x*2), int(y*2), int(w*2), int(h*2)
            face_roi = frame[max(0, y-15):y+h+15, max(0, x-15):x+w+15]
            if face_roi.size == 0:
                continue

            new_emb = get_face_embedding(face_roi)
            if new_emb is None:
                continue

            for u in users:
                norm_prod = np.linalg.norm(new_emb) * np.linalg.norm(u["embedding"])
                if norm_prod == 0:
                    continue
                dist = np.dot(new_emb, u["embedding"]) / norm_prod
                if dist > 0.70:
                    print(json.dumps({"status": "success", "id": u["id"], "name": u["name"], "role": u["role"]}), flush=True)
                    matched = True
                    break
            if matched:
                break
        if matched:
            break

    cap.release()

    if not matched:
        print(json.dumps({"status": "failed", "message": "Face not recognized. Please try again."}), flush=True)


if __name__ == "__main__":
    init_db()
    if len(sys.argv) < 2:
        print("Usage: python face_auth.py <register|verify> [name] [role]")
        sys.exit(1)

    command = sys.argv[1]
    if command == "register":
        if len(sys.argv) < 4:
            print("Usage: python face_auth.py register <name> <role>")
        else:
            register_user(sys.argv[2], sys.argv[3])
    elif command == "verify":
        verify_user_prewarmed()
