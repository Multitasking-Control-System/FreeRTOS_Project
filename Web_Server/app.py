from flask import Flask, render_template, jsonify
import mysql.connector
import psutil
from datetime import datetime

app = Flask(__name__)

# =====================================================
# MYSQL CONFIGURATION
# =====================================================

DB_HOST = "localhost"
DB_USER = "root"
DB_PASSWORD = "Root@123"
DB_NAME = "sensor_db"


# =====================================================
# MYSQL CONNECTION
# =====================================================

def get_db_connection():

    return mysql.connector.connect(
        host=DB_HOST,
        user=DB_USER,
        password=DB_PASSWORD,
        database=DB_NAME
    )


# =====================================================
# DASHBOARD PAGE
# =====================================================

@app.route("/")
def dashboard():

    return render_template("index.html")


# =====================================================
# SENSOR DATA API
# =====================================================

@app.route("/api/data")
def sensor_data():

    try:

        db = get_db_connection()
        cursor = db.cursor(dictionary=True)

        # ---------------------------------------------
        # Latest sensor reading
        # ---------------------------------------------

        cursor.execute("""
            SELECT
                id,
                `timestamp`,
                temperature,
                pressure,
                lux,
                ax,
                ay,
                az,
                heart_rate,
                spo2
            FROM sensor_data
            ORDER BY id DESC
            LIMIT 1
        """)

        latest = cursor.fetchone()


        # ---------------------------------------------
        # Last 30 readings for graph
        # ---------------------------------------------

        cursor.execute("""
            SELECT
                `timestamp`,
                temperature,
                pressure,
                lux,
                heart_rate,
                spo2
            FROM sensor_data
            ORDER BY id DESC
            LIMIT 30
        """)

        history = cursor.fetchall()

        # Reverse so oldest → newest
        history.reverse()


        cursor.close()
        db.close()


        # ---------------------------------------------
        # CPU usage
        # ---------------------------------------------

        cpu_usage = psutil.cpu_percent(interval=0.1)


        # ---------------------------------------------
        # Database status
        # ---------------------------------------------

        database_status = True


        # ---------------------------------------------
        # Data flow status
        # ---------------------------------------------

        data_status = False

        if latest:

            last_time = latest["timestamp"]

            if isinstance(last_time, datetime):

                difference = (
                    datetime.now() - last_time
                ).total_seconds()

                # Data considered live if received
                # within last 10 seconds

                if difference <= 10:
                    data_status = True


        return jsonify({

            "latest": latest,

            "history": history,

            "cpu_usage": cpu_usage,

            "database_connected": database_status,

            "mqtt_connected": data_status,

            "system_running": data_status

        })


    except Exception as e:

        return jsonify({

            "error": str(e),

            "database_connected": False,

            "mqtt_connected": False,

            "system_running": False

        }), 500


# =====================================================
# RUN FLASK
# =====================================================

if __name__ == "__main__":

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )