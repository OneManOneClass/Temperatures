import os
import sys
import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from datetime import datetime
import controller.db_handler as db

bg_colour = "#3d6466"


class GUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Temperature and Forecast Data")
        self.root.geometry("1000x600")

        # Set the icon for the main window
        icon_path = os.path.join(os.path.dirname(__file__), "../media/temperatures.ico")
        self.root.iconbitmap(icon_path)

        style = ttk.Style()
        style.configure("Custom.TLabel",
                        font=("Arial", 12),
                        foreground="black",
                        background="white",
                        padding=5)

        style.configure("Custom.TEntry",
                        font=("Arial", 12),
                        foreground="black",
                        background="white",
                        padding=5)

        self.temperature_label = ttk.Label(self.root, text="Arduino Temperature: ", style="Custom.TLabel")
        self.temperature_label.pack(pady=10, anchor=tk.NW)

        self.forecast_label = ttk.Label(self.root, text="Forecast Temperature: ", style="Custom.TLabel")
        self.forecast_label.pack(pady=10, anchor=tk.NW)

        self.refresh_button = ttk.Button(self.root, text="Refresh", command=self.refresh_data)
        self.refresh_button.pack(pady=10, anchor=tk.NW)

        self.start_datetime_label = ttk.Label(self.root, text="Start Date and Time (YYYY-MM-DD HH:MM:SS):",
                                              style="Custom.TLabel")
        self.start_datetime_label.pack(pady=5)

        self.start_datetime_entry = ttk.Entry(self.root)
        self.start_datetime_entry.pack(pady=5)

        self.end_datetime_label = ttk.Label(self.root, text="End Date and Time (YYYY-MM-DD HH:MM:SS):",
                                            style="Custom.TLabel")
        self.end_datetime_label.pack(pady=5)

        self.end_datetime_entry = ttk.Entry(self.root)
        self.end_datetime_entry.pack(pady=5)

        self.plot_button = ttk.Button(self.root, text="Plot", command=self.plot_data)
        self.plot_button.pack(pady=10)

        self.plot_canvas = None

        self.refresh_data()

        self.root.protocol("WM_DELETE_WINDOW", self.root.destroy)  # WINDOW BEGONE

    def refresh_data(self):
        current_data = db.getCurrentTemperature()
        if current_data:
            self.temperature_label.config(text=f"Arduino Temperature: {current_data.temp_value:.2f}")
            if current_data.forecast:
                self.forecast_label.config(text=f"Forecast Temperature: {current_data.forecast.forecast_value:.2f}")
            else:
                self.forecast_label.config(text="Forecast: no data!")
        else:
            self.temperature_label.config(text="No data available")
            self.forecast_label.config(text="")

    def plot_data(self):
        start_datetime_str = self.start_datetime_entry.get()
        end_datetime_str = self.end_datetime_entry.get()

        if start_datetime_str and end_datetime_str:
            try:
                start_datetime = datetime.strptime(start_datetime_str, '%Y-%m-%d %H:%M:%S')
                end_datetime = datetime.strptime(end_datetime_str, '%Y-%m-%d %H:%M:%S')

                if start_datetime <= end_datetime:
                    data = db.getPlotRecords(start_datetime, end_datetime)

                    if data:
                        arduino_temp_values = [record.temp_value for record in data]
                        forecast_temp_values = [record.forecast.forecast_value if record.forecast else None for record
                                                in data]
                        time_values = [record.temp_date for record in data]

                        if self.plot_canvas:
                            self.plot_canvas.get_tk_widget().pack()

                        fig, ax = plt.subplots(figsize=(12, 6))  # Update the figure size here
                        ax.plot(time_values, arduino_temp_values, label="Arduino Temperature")
                        ax.plot(time_values, forecast_temp_values, label="Forecast Temperature")

                        ax.set_xlabel("Time")
                        ax.set_ylabel("Temperature")
                        ax.legend()

                        self.plot_canvas = FigureCanvasTkAgg(fig, master=self.root)
                        self.plot_canvas.get_tk_widget().pack(pady=10)
                    else:
                        self.show_error_message("No data available for the selected time range!")
                else:
                    self.show_error_message(
                        "Invalid time range!")
            except ValueError:
                self.show_error_message("Invalid date and time format. (YYYY-MM-DD HH:MM:SS)")
        else:
            self.show_error_message("Please enter both start and end dates.")

    def show_error_message(self, message):
        tk.messagebox.showerror("Error", message)

    def on_close(self):
        # Destroy the root window and exit the application gracefully
        self.root.destroy()
        sys.exit()


if __name__ == "__main__":
    root = tk.Tk()
    gui = GUI(root)

    root.configure(bg=bg_colour)
    root.mainloop()
