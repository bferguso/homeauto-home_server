create table ha_station_log (
   "remote_address" VARCHAR(15) NOT NULL,
   "node_location" VARCHAR(20) NOT NULL,
   "log_timestamp" TIMESTAMP WITHOUT TIME ZONE DEFAULT NOW(),
   "log_text" text
);

create index ha_sl_idx1 on ha_station_log (remote_address, node_location);
create index ha_sl_idx2 on ha_station_log (log_timestamp);
