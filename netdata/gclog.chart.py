import re

from bases.FrameworkServices.LogService import LogService

ORDER = ['cpm']
CHARTS = {
    'cpm': {
        'options': ['cpm', 'CPM (Clicks Per Minute)', 'cpm', None, 'line', False],
        'lines': [
            ['cpm', 'cpm', 'absolute'],
            ['samples', 'samples', 'absolute'],
        ],
    },
}


EXAMPLE_LOG = '''
Feb 11 04:28:05 yak-ubuntower gclog[332610]: CPM: 12 (= 61/5), Timestamp: Sat Feb 11 09:28:00 2023
'''

RGX_CPM = re.compile(r'CPM: (?P<cpm>\d+) \(= (?P<sum>\d+)/(?P<samples>\d+)\), Timestamp: (.*)')


class Service(LogService):
    def __init__(self, configuration=None, name=None):
        configuration = configuration if configuration is not None else {}
        configuration.setdefault('path', '/var/log/syslog')

        super().__init__(configuration=configuration, name=name)

        self.order = ORDER
        self.definitions = CHARTS

    def _get_data(self):
        lines = self._get_raw_data()
        for line in lines:
            if 'gclog[' not in line:
                continue

            match = RGX_CPM.search(line)
            if not match:
                continue

            cpm = int(match.group('cpm'))
            samples = int(match.group('samples'))

            return {
                'cpm': cpm,
                'samples': samples,
            }
