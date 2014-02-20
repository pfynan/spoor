struct LightCoordinates {
    1: i16 phi,
    2: i16 theta
}

enum PointMode {
    MANUAL,
    AUTOMATIC
}


service AutoNoLight {
    void setMode(1: PointMode mode),
    PointMode getMode(),
    void setPos(1: i16 target),
    Coordinates getActualPos()
}

