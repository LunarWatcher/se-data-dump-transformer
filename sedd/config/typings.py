from typing import TypedDict, Literal


class SEDDNotificationsConfig(TypedDict):
    provider: Literal['native'] | None


class SEDDUboUserFiltersSettings(TypedDict):
    enabled: bool
    trusted: bool
    toOverwrite: list[str]


class SEDDUboSettings(TypedDict):
    userSettings: dict[str, str | bool | int]
    hiddenSettings: dict[str, str | bool | int]
    selectedFilterLists: list[str]
    whitelist: list[str]
    dynamicFilters: dict[Literal['toAdd'] | Literal['toRemove'], list[str]]
    userFilters: SEDDUboUserFiltersSettings


class SEDDUboConfig(TypedDict):
    download_url: str
    settings: SEDDUboSettings | None
