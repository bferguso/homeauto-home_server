create user homeauto password 'homeauto';
create database homeauto with owner = homeauto;
create schema homeauto;
\c homeauto
grant all on schema homeauto to homeauto;

create table homeauto.ha_environment_log (
    sensor_location VARCHAR(20),
    temperature numeric(5,2),
    humidity numeric(5,2),
    pressure numeric(6,2),
    altitude numeric(5,2),
    heat_index numeric(5,2),
    sample_timestamp timestamp default current_timestamp
);

create index ha_env_log_location_idx on homeauto.ha_environment_log(sensor_location NULLS FIRST);
create index ha_env_log_sample_time_idx on homeauto.ha_environment_log(sample_timestamp NULLS FIRST);

create table homeauto.ha_remote_devices (
    remote_address varchar(15) not null,
    sensor_location varchar(20) not null,
    device_active boolean default 'Y' not null,
    last_seen_timestamp timestamp default current_timestamp
);