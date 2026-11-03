// api.mjs
const WORKER_URL = "https://superdb-api.superbem.workers.dev";
const GOOGLE_CLIENT_ID = "196492632823-0tidhu363lcsu96ean7ia0v9iavob1ij.apps.googleusercontent.com";

// URL de redirecionamento (página atual onde a app está alojada)
// Remove qualquer barra final para garantir consistência com o Google Console
let REDIRECT_URI = window.location.origin + window.location.pathname;
// let rawUri = window.location.origin + window.location.pathname;
// if (rawUri.endsWith('/') && rawUri.length > 8) { // Mantém a barra apenas se for a raiz pura (ex: http://127.0.0.1:8080/)
//     // Podes optar por remover a barra final se registaste o URL sem barra no Google Console:
//     rawUri = rawUri.replace(/\/$/, "");
// }
// const REDIRECT_URI = rawUri;


// Função para detetar e capturar o access_token quando o Google redireciona de volta para a app
async function handleOAuthRedirect() {
    const hash = window.location.hash;
    if (!hash.includes('access_token')) return false;

    try {
        const params = new URLSearchParams(hash.substring(1));
        const accessToken = params.get('access_token');
        
        // Captura o URL original que guardámos no state
        const targetUrl = params.get('state'); 

        if (accessToken) {
            console.log("Access Token detetado! A autenticar com o Worker...");
            
            // 1. Envia o token para o seu Cloudflare Worker primeiro
            await authenticateWithWorker({ access_token: accessToken });

            // 2. Após autenticar, redireciona o utilizador de volta para a página original
            if (targetUrl && targetUrl !== window.location.href) {
                window.location.href = targetUrl;
            } else {
                // Se já estiver na página certa, apenas limpa o hash do URL
                window.history.replaceState({}, document.title, window.location.pathname);
            }
            return true;
        }
    } catch (err) {
        console.error("Erro ao processar o redirecionamento OAuth:", err);
    }
    return false;
}

// Função para verificar se já existe uma sessão ativa ou um retorno de OAuth ao abrir a página
export async function initSession() {
    // 1. Verifica se viemos de um redirecionamento do Google OAuth
    const handledRedirect = await handleOAuthRedirect();
    if (handledRedirect) return getCurrentUser();

    // 2. Caso contrário, verifica a sessão guardada no localStorage
    const token = localStorage.getItem("app_token");
    const userJson = localStorage.getItem("user_info");

    if (token && userJson) {
        try {
            const user = JSON.parse(userJson);
            console.log("Sessão existente encontrada para:", user.email);

            // Dispara o evento para a interface / C++ atualizar automaticamente
            window.dispatchEvent(new CustomEvent('user-logged-in', { detail: user }));
            if (typeof window.onLoginSuccess === 'function') {
                window.onLoginSuccess(user);
            }

            return user;
        } catch (e) {
            console.error("Erro ao ler dados da sessão anterior:", e);
            logout(); // Limpa dados corrompidos
        }
    }
    return null;
}

// Executa automaticamente assim que o script é carregado
initSession();

export function triggerGoogleLogin() {
    console.log("A redirecionar para o Google OAuth (Fluxo compatível com COOP strict e Wasm)...");

    // Constrói o URL do endpoint de autenticação do Google por redirecionamento
    const authUrl = `https://accounts.google.com/o/oauth2/v2/auth?` +
        `client_id=${encodeURIComponent(GOOGLE_CLIENT_ID)}` +
        `&redirect_uri=${encodeURIComponent(window.location.origin)}` +
        `&response_type=token` +
        `&scope=${encodeURIComponent('openid email profile')}` +
        `&state=${encodeURIComponent(window.location.origin+window.location.pathname)}`;

    // Redireciona a janela inteira para o Google
    window.location.href = authUrl;
}

async function authenticateWithWorker(payload) {
    console.log("7. A enviar access_token para o Cloudflare Worker...", payload);
    try {
        const res = await fetch(`${WORKER_URL}/auth/google`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(payload)
        });

        console.log("7b. Status HTTP da resposta do Worker:", res.status);
        const data = await res.json();
        console.log("8. Resposta JSON completa do Worker:", data);
        
        if (data.success) {
            localStorage.setItem("app_token", data.token);
            localStorage.setItem("user_info", JSON.stringify(data.user));
            
            window.dispatchEvent(new CustomEvent('user-logged-in', { detail: data.user }));
            if (typeof window.onLoginSuccess === 'function') {
                window.onLoginSuccess(data.user);
            }

            console.log("Login efetuado com sucesso absoluto!");
            alert("Login bem-sucedido! Bem-vindo, " + data.user.name);
        } else {
            console.error("O Worker rejeitou o login:", data.error);
            alert("Erro no login (Servidor): " + (data.error || "Desconhecido"));
        }
    } catch (err) {
        console.error("EXCEÇÃO DE REDE AO FALAR COM O WORKER:", err);
        alert("Erro de comunicação com o servidor de autenticação.");
    }
}

export async function executeSql(sql, params = []) {
    const token = localStorage.getItem("app_token");
    const headers = { "Content-Type": "application/json" };
    if (token) headers["Authorization"] = `Bearer ${token}`;

    const res = await fetch(`${WORKER_URL}/sql`, {
        method: "POST",
        headers,
        body: JSON.stringify({ sql, params })
    });
    
    return await res.json();
}

export function getCurrentUser() {
    const user = localStorage.getItem("user_info");
    return user ? JSON.parse(user) : null;
}

export function logout() {
    localStorage.removeItem("app_token");
    localStorage.removeItem("user_info");
    location.reload();
}

window.triggerGoogleLogin = triggerGoogleLogin;
window.logout = logout;
window.getCurrentUser = getCurrentUser;
window.executeSql = executeSql;