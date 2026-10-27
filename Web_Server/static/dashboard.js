let chart;


/* =====================================================
   FORMAT TIME
===================================================== */

function formatTime(timestamp) {

    if (!timestamp)
        return "--";

    const date = new Date(
        timestamp
    );

    return date.toLocaleTimeString();

}


/* =====================================================
   UPDATE DASHBOARD
===================================================== */

async function updateDashboard() {

    try {

        const response =
            await fetch("/api/data");

        const data =
            await response.json();


        if (data.error) {

            console.error(data.error);

            return;

        }


        const latest =
            data.latest;


        if (!latest)
            return;


        /* =============================================
           TOP CARDS
        ============================================= */

        document.getElementById(
            "temperature"
        ).innerText =
            latest.temperature.toFixed(2)
            + " °C";


        document.getElementById(
            "pressure"
        ).innerText =
            latest.pressure.toFixed(2)
            + " hPa";


        document.getElementById(
            "lux"
        ).innerText =
            latest.lux.toFixed(2)
            + " Lux";


        document.getElementById(
            "heart_rate"
        ).innerText =
            latest.heart_rate
            + " BPM";


        document.getElementById(
            "spo2"
        ).innerText =
            latest.spo2
            + " %";


        document.getElementById(
            "cpu_usage"
        ).innerText =
            data.cpu_usage.toFixed(1)
            + " %";


        /* =============================================
           TABLE
        ============================================= */

        document.getElementById(
            "tableLux"
        ).innerText =
            latest.lux.toFixed(2);


        document.getElementById(
            "tableTemp"
        ).innerText =
            latest.temperature.toFixed(2);


        document.getElementById(
            "tablePressure"
        ).innerText =
            latest.pressure.toFixed(2);


        document.getElementById(
            "tableAX"
        ).innerText =
            latest.ax;


        document.getElementById(
            "tableAY"
        ).innerText =
            latest.ay;


        document.getElementById(
            "tableAZ"
        ).innerText =
            latest.az;


        document.getElementById(
            "tableHR"
        ).innerText =
            latest.heart_rate;


        document.getElementById(
            "tableSpO2"
        ).innerText =
            latest.spo2;


        /* =============================================
           TIME
        ============================================= */

        const time =
            formatTime(latest.timestamp);


        document.getElementById(
            "updateTime"
        ).innerText =
            "Time: " + time;


        document.getElementById(
            "timeLux"
        ).innerText = time;


        document.getElementById(
            "timeTemp"
        ).innerText = time;


        document.getElementById(
            "timePressure"
        ).innerText = time;


        document.getElementById(
            "timeMPU"
        ).innerText = time;


        document.getElementById(
            "timeHR"
        ).innerText = time;


        /* =============================================
           STATUS
        ============================================= */

        const mqttOK =
            data.mqtt_connected;


        const dbOK =
            data.database_connected;


        document.getElementById(
            "mqttStatus"
        ).innerText =
            mqttOK
                ? "Connected"
                : "Disconnected";


        document.getElementById(
            "databaseStatus"
        ).innerText =
            dbOK
                ? "Connected"
                : "Disconnected";


        document.getElementById(
            "systemStatus"
        ).innerText =
            data.system_running
                ? "Running"
                : "No Data";


        document.getElementById(
            "mqttDot"
        ).className =
            mqttOK
                ? "dot green"
                : "dot";


        document.getElementById(
            "systemDot"
        ).className =
            data.system_running
                ? "dot green"
                : "dot";


        /* =============================================
           ALERTS
        ============================================= */

        let alerts = [];


        if (latest.temperature >= 30) {

            alerts.push(
                "High Temperature"
            );

        }


        if (latest.spo2 > 0 &&
            latest.spo2 < 90) {

            alerts.push(
                "Low SpO2"
            );

        }


        if (latest.heart_rate > 120) {

            alerts.push(
                "High Heart Rate"
            );

        }


        const alertElement =
            document.getElementById(
                "alertMessage"
            );


        if (alerts.length > 0) {

            alertElement.innerText =
                alerts.join(", ");

            alertElement.className =
                "warning";

        }
        else {

            alertElement.innerText =
                "No abnormal values";

            alertElement.className =
                "no-alert";

        }


        /* =============================================
           CHART
        ============================================= */

        updateChart(
            data.history
        );

    }

    catch(error) {

        console.error(
            "Dashboard error:",
            error
        );

    }

}


/* =====================================================
   CHART
===================================================== */

function updateChart(history) {

    const selected =
        document.getElementById(
            "chartSelect"
        ).value;


    const labels =
        history.map(
            item =>
                formatTime(
                    item.timestamp
                )
        );


    const values =
        history.map(
            item =>
                item[selected]
        );


    if (!chart) {

        const ctx =
            document.getElementById(
                "sensorChart"
            );


        chart = new Chart(
            ctx,
            {

                type: "line",

                data: {

                    labels: labels,

                    datasets: [{

                        label:
                            selected,

                        data: values,

                        borderWidth: 2,

                        tension: 0.3,

                        fill: false

                    }]

                },

                options: {

                    responsive: true,

                    maintainAspectRatio: false,

                    animation: false,

                    scales: {

                        y: {

                            beginAtZero: false

                        }

                    }

                }

            }

        );

    }

    else {

        chart.data.labels =
            labels;

        chart.data.datasets[0].data =
            values;

        chart.data.datasets[0].label =
            selected;

        chart.update();

    }

}


/* =====================================================
   CHART SELECTION
===================================================== */

document.getElementById(
    "chartSelect"
).addEventListener(
    "change",
    updateDashboard
);


/* =====================================================
   REAL-TIME REFRESH
===================================================== */

updateDashboard();


setInterval(
    updateDashboard,
    2000
);