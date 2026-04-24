import cv2
import sys
import os
import json
import sqlite3
import numpy as np
import io
from deepface import DeepFace

# Force UTF-8 encoding for Windows terminal to handle emojis in logs
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')

# Database setup
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
DB_PATH = os.path.join(SCRIPT_DIR, "faces.db")

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            role TEXT NOT NULL,
            embedding BLOB NOT NULL
        )
    ''')
    conn.commit()
    conn.close()

def capture_face():
    cap = cv2.VideoCapture(0)
    if not cap.isOpened():
        print("ERROR: Could not open webcam. Check if another app is using it.")
        return None
        
    print("LOG: Camera opened. Look at the camera.")
    window_name = "Face ID - Press SPACE to Capture"
    cv2.namedWindow(window_name, cv2.WINDOW_AUTOSIZE)
    cv2.setWindowProperty(window_name, cv2.WND_PROP_TOPMOST, 1)
    
    face_detected = None
    
    while True:
        ret, frame = cap.read()
        if not ret:
            print("ERROR: Failed to grab frame from camera.")
            break
            
        cv2.imshow(window_name, frame)
        
        key = cv2.waitKey(1)
        if key == ord(' '):  # Space to capture
            face_detected = frame
            break
        elif key == 27:  # ESC to cancel
            break
            
    cap.release()
    cv2.destroyAllWindows()
    return face_detected

def register_user(name, role):
    frame = capture_face()
    if frame is None:
        print("ERROR: Capture cancelled.")
        return
        
    try:
        # Generate embedding
        # Set enforce_detection to False if you want to allow images without clear faces (not recommended for security but good for testing)
        results = DeepFace.represent(frame, model_name="Facenet", enforce_detection=False)
        if not results or "embedding" not in results[0]:
            print("ERROR: Face not detected clearly enough.")
            return
            
        embedding = results[0]["embedding"]
        
        # Save to DB
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        # Convert embedding list to bytes
        embedding_blob = np.array(embedding).tobytes()
        cursor.execute("INSERT INTO users (name, role, embedding) VALUES (?, ?, ?)", 
                       (name, role, embedding_blob))
        conn.commit()
        conn.close()
        print(f"SUCCESS: User {name} registered as {role}.")
    except Exception as e:
        print(f"ERROR: {str(e)}")

def verify_user():
    frame = capture_face()
    if frame is None:
        print("ERROR: Capture cancelled.")
        return
        
    try:
        # Generate current embedding
        results = DeepFace.represent(frame, model_name="Facenet", enforce_detection=False)
        if not results or "embedding" not in results[0]:
            print(json.dumps({"status": "error", "message": "Face not detected clearly enough"}))
            return
            
        new_embedding = np.array(results[0]["embedding"])
        
        # Compare with DB
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()
        cursor.execute("SELECT id, name, role, embedding FROM users")
        rows = cursor.fetchall()
        
        best_match = None
        min_dist = 0.5 # Threshold for Facenet
        
        for row in rows:
            user_id, name, role, embedding_blob = row
            stored_embedding = np.frombuffer(embedding_blob)
            
            # Cosine distance
            dist = np.dot(new_embedding, stored_embedding) / (np.linalg.norm(new_embedding) * np.linalg.norm(stored_embedding))
            # Actually dist is cosine similarity here, so higher is better
            
            if dist > 0.7: # Similarity threshold
                print(json.dumps({"status": "success", "id": user_id, "name": name, "role": role}))
                conn.close()
                return
                
        conn.close()
        print(json.dumps({"status": "failed", "message": "No match found"}))
        
    except Exception as e:
        print(json.dumps({"status": "error", "message": str(e)}))

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
        verify_user()
