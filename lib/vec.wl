struct vec4 {
    float[4] v

    this(float x, float y, float z, float w) {
        .v = [x, y, z, w]
    }

    float x() return .v[0]
    float y() return .v[1]
    float z() return .v[2]
    float w() return .v[3]
}
