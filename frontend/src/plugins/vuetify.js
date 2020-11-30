import Vue from 'vue';
import Vuetify from 'vuetify/lib';

Vuetify.config.silent = true;

Vue.use(Vuetify);

export default new Vuetify({
    theme: { dark: true },
});
