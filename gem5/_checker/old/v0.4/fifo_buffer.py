from gem5.components.boards.abstract_board import AbstractBoard
from gem5.components.component import Component

class FIFOBuffer(Component):
    def __init__(self, size):
        super().__init__()
        self.size = size
        self.buffer = []

    def push(self, item):
        if len(self.buffer) < self.size:
            self.buffer.append(item)
        else:
            self.buffer.pop(0)
            self.buffer.append(item)

    def pop(self):
        if self.buffer:
            return self.buffer.pop(0)
        return None

    def is_empty(self):
        return len(self.buffer) == 0

    def is_full(self):
        return len(self.buffer) == self.size