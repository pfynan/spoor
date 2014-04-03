struct Coordinates {
    1: i16 phi,
    2: i16 theta
}

enum PointMode {
    MANUAL,
    AUTOMATIC
}


service Tracking {
    void setMode(1: PointMode mode),
    PointMode getMode(),
    void setPos(1: i16 target),
    Coordinates getActualPos(),
    void setOnOff(1: bool state)
}

