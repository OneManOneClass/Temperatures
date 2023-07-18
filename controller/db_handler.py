from datetime import datetime

from sqlalchemy import and_, extract, desc
from sqlalchemy.orm import sessionmaker, subqueryload

from models.arduino_db import Arduino, Forecast, engine

def newSession():
    Session = sessionmaker(bind=engine)
    session = Session()
    return session


def retrieveAllRecords():

    return newSession().query(Arduino).outerjoin(Forecast).all()


def getCurrentTemperature():
    result = newSession().query(Arduino).order_by(desc(Arduino.temp_date)).options(subqueryload(Arduino.forecast)).first()
    return result


def get_arduino_records(**kwargs):
    query = newSession().query(Arduino)

    date_filters = []  # list to store datetime filters
    other_filters = []  # list to store other filters

    for key, value in kwargs.items():
        if key == 'temp_date':
            date_filters.append(extract('year', Arduino.temp_date) == value.year)
            date_filters.append(extract('month', Arduino.temp_date) == value.month)
            date_filters.append(extract('day', Arduino.temp_date) == value.day)
            date_filters.append(extract('hour', Arduino.temp_date) == value.hour)
        else:
            other_filters.append(getattr(Arduino, key) == value)

    if date_filters:
        date_filter = and_(*date_filters)
        if other_filters:
            query = query.filter(and_(date_filter, *other_filters))
        else:
            query = query.filter(date_filter)
    elif other_filters:
        query = query.filter(and_(*other_filters))

    return query.all()


def getPlotRecords(start_datetime, end_datetime):
    result = newSession().query(Arduino).filter(Arduino.temp_date.between(start_datetime, end_datetime)).all()
    return result

if __name__ == "__main__":
    currentData = getCurrentTemperature()
    print(currentData)

