{
    'targets': [{
        'target_name': 'shmmap',
        'include_dirs': ['../../src'],
        'link_settings': {
            'libraries': ['#LIB_DIR#/libshmmap.a']
        },
        'sources': ['shmmap.cc']
    }]
}
