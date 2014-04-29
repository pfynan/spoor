struct Coordinates {
    1: i16 x,
    2: i16 y
}

enum PointMode {
    MANUAL,
    AUTOMATIC
}

enum LightStatus {
    OVERHEAT,
    ON,
    OFF
}

enum MoveStatus {
    RUNNING,
    UNCAL,
    CALLING,
    SLEEPING,
    TILT_OVERCUR,
    TILT_FAULT,
    PAN_OVERCUR,
    PAN_FAULT
}


service Tracking
    { void setMode(1: PointMode mode)
    , PointMode getMode()

    , void setPos(1: Coordinates coord)
    , void setOnOff(1: bool state)
    , void halt()
    , void sleep()
    , void wake()
    , void setIntensity(1: byte intens)
    , void calibrate()
    , LightStatus getLightStatus()
    , byte getIntensity()
    , MoveStatus getMoveStatus()
    , Coordinates getActualPos()
    }

