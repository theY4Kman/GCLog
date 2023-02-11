from __future__ import annotations

import re
from typing import NamedTuple, Iterable

from bases.FrameworkServices.LogService import LogService


class Options(NamedTuple):
    name: str | None = None
    title: str | None = None
    units: str | None = None
    family: str | None = None
    context: str | None = None
    chart_type: str | None = None
    hidden: bool | None = None


class Line(NamedTuple):
    id: str
    name: str | None = None
    algorithm: str | None = 'absolute'
    multiplier: int | None = 1
    divisor: int | None = 1
    hidden: bool | None = None

    def to_list(self) -> list[str | int | None]:
        return [
            self.id,
            self.name or self.id,
            self.algorithm,
            self.multiplier,
            self.divisor,
            self.hidden,
        ]


class Chart(NamedTuple):
    options: Options
    lines: list[Line]


class Charts:
    def __init__(self, **charts: Chart):
        self._charts = {
            id: {
                'options': [*chart.options],
                'lines': [line.to_list() for line in chart.lines],
            }
            for id, chart in charts.items()
        }

    def __iter__(self):
        yield from self._charts

    def __getitem__(self, key: str) -> dict:
        return self._charts[key]

    def get(self, key: str, default: dict | None = None) -> dict | None:
        return self._charts.get(key, default)


CHARTS = {
    'cpm': {
        'options': ['cpm', 'CPM (Clicks Per Minute)', 'cpm', None, 'line', False],
        'lines': [
            ['cpm', 'cpm', 'absolute'],
            ['samples', 'samples', 'absolute'],
        ],
    },
}
CHARTS = Charts(
    cpm=Chart(
        options=Options(
            name='cpm',
            title='CPM',
            units='cpm',
            chart_type='line',
        ),
        lines=[
            Line('cpm'),
            Line('samples'),
        ],
    ),
)
ORDER = list(CHARTS)


EXAMPLE_LOG = '''
Feb 11 04:28:05 yak-ubuntower gclog[332610]: Instant CPM: 12, Timestamp: Sat Feb 11 09:28:00 2023
'''

RGX_INSTANT_CPM = re.compile(r'Instant CPM: (?P<cpm>\d+), Timestamp: (.*)')


class Service(LogService):
    def __init__(self, configuration=None, name=None):
        configuration = configuration if configuration is not None else {}
        configuration.setdefault('path', '/var/log/syslog')

        super().__init__(configuration=configuration, name=name)

        self.order = ORDER
        self.definitions = CHARTS

    def _get_data(self):
        cpms = list(self._get_cpm_readouts())

        cpm = sum(cpms) / len(cpms) if cpms else None
        samples = len(cpms)

        return {
            'cpm': cpm,
            'samples': samples,
        }

    def _get_cpm_readouts(self) -> Iterable[int]:
        lines = self._get_raw_data()
        for line in lines:
            if 'gclog[' not in line:
                continue

            match = RGX_INSTANT_CPM.search(line)
            if not match:
                continue

            yield int(match.group('cpm'))
