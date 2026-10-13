from flask import Flask, render_template, request
import subprocess
import os

app = Flask(__name__)

@app.route("/", methods=["GET", "POST"])
def index():
    source_code = "int x;\nfloat y;\nx = 10 + 20 * 5;\ny = (x + 30) / 2;"
    tac_output = ""
    symbol_rows = []
    error = ""
    status = "READY"

    if request.method == "POST":
        source_code = request.form.get("code", "")
        
        input_filename = "temp_input.txt"
        with open(input_filename, "w") as f:
            f.write(source_code)

        try:
            result = subprocess.run(
                ["./compiler", input_filename],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=3
            )

            if result.returncode == 0:
                raw_output = result.stdout
                status = "SUCCESS"
                
                tac_lines = []
                for line in raw_output.splitlines():
                    if line.startswith("[TABLE_ROW]"):
                        # Parse: [TABLE_ROW] name | type | scope
                        parts = line.replace("[TABLE_ROW]", "").strip().split("|")
                        if len(parts) >= 3:
                            symbol_rows.append({
                                "name": parts[0].strip(),
                                "type": parts[1].strip(),
                                "scope": parts[2].strip()
                            })
                    else:
                        tac_lines.append(line)
                
                tac_output = "\n".join(tac_lines).strip()
            else:
                error = result.stderr if result.stderr else result.stdout
                status = "ERROR"

        except Exception as e:
            error = str(e)
            status = "ERROR"

        if os.path.exists(input_filename):
            os.remove(input_filename)

    return render_template(
        "index.html", 
        code=source_code, 
        tac_output=tac_output, 
        symbol_rows=symbol_rows, 
        error=error, 
        status=status
    )

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5001, debug=True)