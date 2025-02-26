from flask import Flask, request, jsonify
import subprocess
import os

app = Flask(__name__)

UPLOAD_FOLDER = os.path.dirname(os.path.abspath(__file__))  # Directory corrente
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER

@app.route("/esegui", methods=["POST"])
def esegui():
    if "file" not in request.files:
        return jsonify({"errore": "Nessun file ricevuto!"}), 400
    
    file = request.files["file"]
    filename = file.filename  # Mantiene il nome originale del file
    filepath = os.path.join(app.config["UPLOAD_FOLDER"], filename)
    file.save(filepath)

    opzione1 = request.form.get("opzione1", "")
    opzione2 = request.form.get("opzione2", "")

    # Costruisco il comando con il nome originale del file
    comando = f"./dfa {opzione1} {opzione2} {filename}"

    print("Eseguo comando:", comando)  # Debug

    try:
        subprocess.run(comando, shell=True, check=True)
        return jsonify({"risultato": "ok"})
    
    except subprocess.CalledProcessError as e:
        return jsonify({"errore": f"Errore nell'esecuzione: {e}"}), 500

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
