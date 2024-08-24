# for most sites, dump names correspond to domain name.
# However, due to some subdomain shenanigans, a couple of sites differ:
files_map: dict[str, str] = {
    'alcohol.stackexchange.com': 'beer.stackexchange.com',
    'alcohol.meta.stackexchange.com': 'beer.meta.stackexchange.com',
    'mattermodeling.stackexchange.com': 'materials.stackexchange.com',
    'mattermodeling.meta.stackexchange.com': 'materials.meta.stackexchange.com',
    'communitybuilding.stackexchange.com': 'moderators.stackexchange.com',
    'communitybuilding.meta.stackexchange.com': 'moderators.meta.stackexchange.com',
    'medicalsciences.stackexchange.com': 'health.stackexchange.com',
    'medicalsciences.meta.stackexchange.com': 'health.meta.stackexchange.com',
    'psychology.stackexchange.com': 'cogsci.stackexchange.com',
    'psychology.meta.stackexchange.com': 'cogsci.meta.stackexchange.com',
    'writing.stackexchange.com': 'writers.stackexchange.com',
    'writing.meta.stackexchange.com': 'writers.meta.stackexchange.com',
    'video.stackexchange.com': 'avp.stackexchange.com',
    'video.meta.stackexchange.com': 'avp.meta.stackexchange.com',
    'meta.es.stackoverflow.com': 'es.meta.stackoverflow.com',
    'meta.ja.stackoverflow.com': 'ja.meta.stackoverflow.com',
    'meta.pt.stackoverflow.com': 'pt.meta.stackoverflow.com',
    'meta.ru.stackoverflow.com': 'ru.meta.stackoverflow.com',
}

inverse_files_map: dict[str, str] = {v: k for k, v in files_map.items()}
