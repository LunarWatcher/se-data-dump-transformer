from dataclasses import dataclass

@dataclass
class LastState:
    last_observed_change: float
    last_observed_size: int
