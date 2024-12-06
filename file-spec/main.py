import os
import typing

OUTPUT_FILE_PATH = "intro.phanim"

Vector2 = tuple[float, float]
Vector4 = tuple[float, float, float, float]
COLOR_BLANK: Vector4 = (0.0, 0.0, 0.0, 0.0)

ANIM_ID_COUNTER = 0

class Line:
    def __init__(
        self,
        start: Vector2,
        end: Vector2,
        stroke_width: float = 1.0,
        stroke_color: Vector4 = COLOR_BLANK
    ) -> None:
        global ANIM_ID_COUNTER

        self.id = ANIM_ID_COUNTER
        ANIM_ID_COUNTER += 1
        self.start = start
        self.end = end
        self.stroke_width = stroke_width
        self.stroke_color = stroke_color

    def __str__(self) -> str:
        return f"""Line {{
    id: {self.id},
    start: {self.start},
    end: {self.end},
    stroke_width: {self.stroke_width},
    stroke_color: {self.stroke_color}
}}"""

class Phanim:
    def __init__(self) -> None:
        self.objs: list = []
        self.anims: list = []

    def add_line(self, line: Line) -> None:
        self.objs.append(line)

    def save(self, filepath: str) -> None:
        with open(filepath, "w") as f:
            f.write("%DATA_SECTION%\n")
            for el in self.objs:
                f.write(str(el))

def main():
    phanim = Phanim()

    line1 = Line((100, 100), (500, 500))
    phanim.add_line(line1)


    # Write out to file
    phanim.save(OUTPUT_FILE_PATH)

if __name__ == "__main__":
    main()
