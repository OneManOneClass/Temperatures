# file: create_table.py

from sqlalchemy import create_engine, Column, Float, Integer, DateTime, ForeignKey
from sqlalchemy.orm import declarative_base, relationship
from datetime import datetime
import secretData as s


engine = create_engine(f'mysql+pymysql://{s.ARDUINO_DB_USERNAME}:{s.ARDUINO_DB_PASSWORD}@localhost/{s.ARDUINO_DB_NAME}')
Base = declarative_base()


class Arduino(Base):
    __tablename__ = 'arduino'
    id = Column(Integer, primary_key=True, autoincrement=True)
    temp_value = Column(Float)
    temp_date = Column(DateTime, default=datetime.utcnow)
    latitude = Column(Float)
    longitude = Column(Float)

    forecast = relationship("Forecast", uselist=False, backref="arduino", lazy='joined')

    def __str__(self):
        forecast_str = "Forecast: Not available"
        if self.forecast:
            forecast_str = f"Forecast: {self.forecast.forecast_value:.2f}"

        return (f"{self.id:<10}"
                f"{self.temp_value:<6}"
                f"{self.temp_date.strftime('%Y-%m-%d %H:%M:%S'):<15}    "
                f"{self.latitude:<10.2f}"
                f"{self.longitude:<10.2f}    "
                f"{forecast_str}")


class Forecast(Base):
    __tablename__ = 'forecast'
    id = Column(Integer, primary_key=True, autoincrement=True)
    forecast_value = Column(Float)

    arduino_id = Column(Integer, ForeignKey('arduino.id'))

    def __str__(self):
        # print(f"{self.id:<10}"
        #         f"{self.forecast_value:<10.2f}"
        #         f"{self.arduino_id:<10}")
        return (f"{self.id:<10}"
                f"{self.forecast_value:<10.2f}"
                f"{self.arduino_id:<10}")


Base.metadata.create_all(engine)
