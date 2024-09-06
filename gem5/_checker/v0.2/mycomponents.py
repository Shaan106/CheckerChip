# mycomponents.py

class InstructionBuffer:
    def __init__(self, size):
        self.size = size
        self.buffer = []

    def push(self, instruction):
        if len(self.buffer) >= self.size:
            raise OverflowError("Instruction buffer overflow")
        self.buffer.append(instruction)

    def pop(self):
        if len(self.buffer) == 0:
            raise IndexError("Instruction buffer underflow")
        return self.buffer.pop(0)

    def is_empty(self):
        return len(self.buffer) == 0

    def is_full(self):
        return len(self.buffer) >= self.size

    def clear(self):
        self.buffer.clear()